#ifdef DEBUG_SERVER

#include "DebugServer.h"

#include "Inspection/DebugJsonUtil.h"
#include "Inspection/StatusInspector.h"
#include "Inspection/SceneInspector.h"

#include "Logger/Log.h"

//Keep the platform sockets headers contained to this translation unit.
#include "cpp-httplib/httplib.h"

#include <string>
#include <cstdlib>

namespace AV{
    //A heap-allocated result shared between the HTTP handler and the queued closure.
    //Owning it via shared_ptr means a timed-out handler can return while the closure
    //(if the pump already claimed it) still writes into a live object rather than a
    //dangling stack frame.
    struct QueryResult{
        rapidjson::Document doc;
        int status = 200;
    };

    static const char* HOST = "127.0.0.1";

    DebugServer::DebugServer() = default;

    DebugServer::~DebugServer(){
        shutdown();
    }

    bool DebugServer::initialise(int port){
        mPort = port;
        mStartTime = std::chrono::steady_clock::now();

        mServer.reset(new httplib::Server());
        _registerRoutes();

        if(!mServer->bind_to_port(HOST, mPort)){
            AV_ERROR("Debug server failed to bind to {}:{} (port in use?). The server will not run.", HOST, mPort);
            mServer.reset();
            return false;
        }

        mServerThread = std::thread([this]{
            mServer->listen_after_bind();
        });

        AV_INFO("Debug server listening on {}:{}", HOST, mPort);
        return true;
    }

    void DebugServer::pumpMainThread(){
        if(mShutdown.load()) return;
        mQueue.pump();
    }

    void DebugServer::shutdown(){
        //Idempotent: safe to call from both an explicit shutdown and the destructor.
        if(mShutdown.exchange(true)) return;

        //1. Wake any handler blocked on the queue so httplib::Server::stop() (which waits
        //   for in-flight handlers) cannot deadlock against a request awaiting a pump.
        mQueue.shutdown();

        //2. Stop listening and finish in-flight handlers.
        if(mServer){
            mServer->stop();
        }

        //3. Join the listen thread.
        if(mServerThread.joinable()){
            mServerThread.join();
        }

        mServer.reset();
    }

    void DebugServer::_registerRoutes(){
        //Helper that marshals a query onto the main thread and writes the JSON response.
        auto runQuery = [this](httplib::Response& res, QueryFunction queryFn){
            auto result = std::make_shared<QueryResult>();
            bool ok = mQueue.execute([result, queryFn]{
                queryFn(result->doc, result->status);
            }, REQUEST_TIMEOUT_MS);

            if(!ok){
                res.status = 503;
                res.set_content(
                    DebugJsonUtil::errorBody("engine did not service the request (paused or shutting down)"),
                    "application/json");
                return;
            }

            res.status = result->status;
            res.set_content(DebugJsonUtil::toString(result->doc), "application/json");
        };

        //GET /api — self-describing endpoint catalog.
        mServer->Get("/api", [runQuery](const httplib::Request&, httplib::Response& res){
            runQuery(res, [](rapidjson::Document& doc, int&){
                rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
                doc.SetObject();
                doc.AddMember("engine", "avEngine", allocator);
                doc.AddMember("apiVersion", 1, allocator);
                doc.AddMember("readOnly", true, allocator);

                rapidjson::Value endpoints(rapidjson::kArrayType);
                auto addEndpoint = [&](const char* path, const char* desc){
                    rapidjson::Value e(rapidjson::kObjectType);
                    e.AddMember("path", rapidjson::Value(path, allocator), allocator);
                    e.AddMember("desc", rapidjson::Value(desc, allocator), allocator);
                    endpoints.PushBack(e, allocator);
                };
                addEndpoint("/api", "This endpoint catalog.");
                addEndpoint("/api/status", "Engine liveness summary: version, uptime, render system, window, fps.");
                addEndpoint("/api/scene?root=<name>&depth=<n>&max=<n>", "Ogre scene graph dump. Defaults depth=3, max=500.");
                addEndpoint("/api/scene/node/<name>", "Single scene node deep dive: transform, parent chain, attached objects.");
                doc.AddMember("endpoints", endpoints, allocator);
            });
        });

        //GET /api/status
        mServer->Get("/api/status", [this, runQuery](const httplib::Request&, httplib::Response& res){
            double uptime = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - mStartTime).count();
            runQuery(res, [uptime](rapidjson::Document& doc, int&){
                StatusInspector::writeStatus(doc, uptime);
            });
        });

        //GET /api/scene?root=<name>&depth=<n>&max=<n>
        mServer->Get("/api/scene", [runQuery](const httplib::Request& req, httplib::Response& res){
            std::string root = req.has_param("root") ? req.get_param_value("root") : "";
            int depth = req.has_param("depth") ? std::atoi(req.get_param_value("depth").c_str()) : 3;
            int maxNodes = req.has_param("max") ? std::atoi(req.get_param_value("max").c_str()) : 500;
            if(depth < 0) depth = 0;
            if(maxNodes < 1) maxNodes = 1;

            runQuery(res, [root, depth, maxNodes](rapidjson::Document& doc, int& status){
                SceneInspector::writeSceneTree(doc, status, root, depth, maxNodes);
            });
        });

        //GET /api/scene/node/<name>
        mServer->Get(R"(/api/scene/node/(.+))", [runQuery](const httplib::Request& req, httplib::Response& res){
            std::string name = req.matches[1];
            runQuery(res, [name](rapidjson::Document& doc, int& status){
                SceneInspector::writeNodeDetail(doc, status, name);
            });
        });
    }
}

#endif
