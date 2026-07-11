#ifdef DEBUG_SERVER

#include "DebugServer.h"

#include "Inspection/DebugJsonUtil.h"
#include "Inspection/StatusInspector.h"
#include "Inspection/SceneInspector.h"
#include "Inspection/RenderInspector.h"
#include "Render/FrameCapture.h"
#include "Eval/ScriptEvaluator.h"

#include "Scripting/ScriptVM.h"

#include "Logger/Log.h"

//Keep the platform sockets headers contained to this translation unit.
#include "cpp-httplib/httplib.h"

#include <string>
#include <cstdlib>
#include <cstdio>
#include <algorithm>

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
        mFrameCapture.reset(new FrameCapture());
        mFrameCapture->initialise();
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

        //1. Wake any handler blocked on the queue or on a pending frame capture, so
        //   httplib::Server::stop() (which waits for in-flight handlers) cannot deadlock
        //   against a request awaiting the main thread.
        mQueue.shutdown();
        if(mFrameCapture) mFrameCapture->shutdown();

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
                //POST /api/eval can mutate engine state; this is a trusted local dev tool.
                doc.AddMember("readOnly", false, allocator);

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
                addEndpoint("/api/render/frame?form=stats|grid|ascii|png&w=<n>&h=<n>&maxDim=<n>&region=x,y,w,h",
                    "Capture the rendered frame as text: stats (default, colour/luminance summary), grid (hex cell colours), ascii (luminance art), png (base64).");
                addEndpoint("POST /api/eval {\"code\": \"<squirrel>\", \"timeoutMs\": <n>}",
                    "Compile and run a Squirrel snippet on the main thread with full engine script API access. Bare expressions return their value; use 'return' in multi-statement snippets. Do not eval unbounded loops: the engine cannot interrupt them.");
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

        //GET /api/render/frame?form=stats|grid|ascii|png&w=&h=&maxDim=&region=x,y,w,h
        //Unlike the other endpoints this does not use the main-thread queue: the capture
        //must happen mid-frame (frameRenderingQueued, before the swap presents the
        //drawable), and the pixel processing afterwards is pure CPU work safe on this
        //HTTP thread.
        mServer->Get("/api/render/frame", [this](const httplib::Request& req, httplib::Response& res){
            RenderInspector::FrameParams params;
            if(req.has_param("form")) params.form = req.get_param_value("form");
            if(req.has_param("w")) params.w = std::atoi(req.get_param_value("w").c_str());
            if(req.has_param("h")) params.h = std::atoi(req.get_param_value("h").c_str());
            if(req.has_param("maxDim")) params.maxDim = std::atoi(req.get_param_value("maxDim").c_str());
            if(req.has_param("region")){
                const std::string region = req.get_param_value("region");
                float x, y, w, h;
                if(sscanf(region.c_str(), "%f,%f,%f,%f", &x, &y, &w, &h) == 4){
                    params.hasRegion = true;
                    params.regionX = x;
                    params.regionY = y;
                    params.regionW = w;
                    params.regionH = h;
                }else{
                    res.status = 400;
                    res.set_content(DebugJsonUtil::errorBody("region must be four comma-separated floats: x,y,w,h"), "application/json");
                    return;
                }
            }
            //Validate the form before capturing so a bad request never costs a readback.
            if(!RenderInspector::validForm(params.form)){
                res.status = 400;
                res.set_content(DebugJsonUtil::errorBody("unknown form '" + params.form + "'; expected stats, grid, ascii or png"), "application/json");
                return;
            }

            CapturedFrame frame;
            std::string error;
            if(!mFrameCapture->requestCapture(frame, error, REQUEST_TIMEOUT_MS)){
                res.status = 503;
                res.set_content(DebugJsonUtil::errorBody("capture failed: " + error), "application/json");
                return;
            }

            rapidjson::Document doc;
            int status = 200;
            RenderInspector::writeFrame(doc, status, params, frame);
            res.status = status;
            res.set_content(DebugJsonUtil::toString(doc), "application/json");
        });

        //POST /api/eval {"code": "<squirrel>", "timeoutMs": <n>}
        //Runs on the main thread via the queue like the inspection endpoints, but with a
        //caller-adjustable timeout since legitimate snippets may do real work.
        mServer->Post("/api/eval", [this](const httplib::Request& req, httplib::Response& res){
            rapidjson::Document body;
            body.Parse(req.body.c_str());
            if(body.HasParseError() || !body.IsObject() || !body.HasMember("code") || !body["code"].IsString()){
                res.status = 400;
                res.set_content(DebugJsonUtil::errorBody("body must be JSON: {\"code\": \"<squirrel>\"}"), "application/json");
                return;
            }
            const std::string code = body["code"].GetString();
            uint32_t timeoutMs = REQUEST_TIMEOUT_MS;
            if(body.HasMember("timeoutMs") && body["timeoutMs"].IsUint()){
                timeoutMs = std::min(body["timeoutMs"].GetUint(), 60000u);
            }

            auto result = std::make_shared<QueryResult>();
            const bool serviced = mQueue.execute([result, code]{
                HSQUIRRELVM vm = ScriptVM::getVMForDebugServer();
                if(!vm){
                    result->status = 503;
                    result->doc.SetObject();
                    result->doc.AddMember("error", "script vm not available", result->doc.GetAllocator());
                    return;
                }
                ScriptEvaluator::eval(vm, code, result->doc);
            }, timeoutMs);

            if(!serviced){
                res.status = 503;
                res.set_content(
                    DebugJsonUtil::errorBody("engine did not service the request (paused, shutting down, or the snippet is still running — an unbounded eval hangs the engine)"),
                    "application/json");
                return;
            }

            res.status = result->status;
            res.set_content(DebugJsonUtil::toString(result->doc), "application/json");
        });
    }
}

#endif
