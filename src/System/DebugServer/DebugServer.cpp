#ifdef DEBUG_SERVER

#include "DebugServer.h"

#include "Inspection/DebugJsonUtil.h"
#include "Inspection/StatusInspector.h"
#include "Inspection/SceneInspector.h"
#include "Inspection/RenderInspector.h"
#include "Inspection/InputInspector.h"
#include "Inspection/GuiInspector.h"
#ifdef SCRIPT_PROFILER
    #include "Inspection/ProfilerInspector.h"
    #include "Scripting/Profiler/ScriptProfiler.h"
#endif
#include "Render/FrameCapture.h"
#include "Render/FrameStore.h"
#include "Render/ImageOps.h"
#include "Eval/ScriptEvaluator.h"
#include "Input/InputPlayback.h"

#include "Scripting/ScriptVM.h"

#include "Logger/Log.h"

#include "httplib.h"

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
        mFrameStore.reset(new FrameStore());
        mInputPlayback.reset(new InputPlayback());
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
        //Tick input lifetimes first, then service requests: a spoof created by a queued
        //closure this frame should begin its countdown next frame, not this one.
        if(mInputPlayback) mInputPlayback->update();
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

        //4. Release any spoofed input so the engine isn't left with stuck keys/buttons.
        //   Safe here: shutdown() runs on the main thread before world teardown.
        if(mInputPlayback) mInputPlayback->clear();

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
                //2 added the script profiler endpoints.
                doc.AddMember("apiVersion", 2, allocator);
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
                addEndpoint("/api/render/hash", "Perceptual dHash of the live frame. Poll it to wait for the screen to settle.");
                addEndpoint("POST /api/render/snapshot?name=<slot>", "Capture now and store it under a name for later comparison.");
                addEndpoint("/api/render/snapshots", "List stored snapshot names.");
                addEndpoint("/api/render/compare?a=<slot>&b=<slot|live>&w=<n>&h=<n>&threshold=<f>",
                    "Diff two frames: hamming distance, changed fraction, changed cells and the bounding box of the change.");
                addEndpoint("/api/render/find?rgb=<hex>&tolerance=<n>&region=x,y,w,h",
                    "Locate connected regions of a colour; returns normalised centroids and bounding boxes.");
                addEndpoint("POST /api/eval {\"code\": \"<squirrel>\", \"timeoutMs\": <n>}",
                    "Compile and run a Squirrel snippet on the main thread with full engine script API access. Bare expressions return their value; use 'return' in multi-statement snippets. Do not eval unbounded loops: the engine cannot interrupt them.");
                addEndpoint("/api/input/actions", "List the project's input action sets and action names, for use with /api/input/action.");
                addEndpoint("POST /api/input/action {\"action\":\"<name>\",\"type\":\"button|axis\",\"value\":<bool>,\"x\":<f>,\"y\":<f>,\"frames\":<n>}",
                    "Inject a button or stick/axis action. frames holds it for N frames (omit or -1 to hold until released).");
                addEndpoint("POST /api/input/mouse {\"button\":<0-2>,\"pressed\":<bool>,\"frames\":<n>} | {\"moveTo\":[x,y]}",
                    "Press/release a mouse button or warp the pointer to a normalised window position.");
                addEndpoint("POST /api/input/clear", "Release everything currently being spoofed.");
                addEndpoint("/api/input/state", "What input the debug server is currently spoofing.");
                addEndpoint("/api/gui/tree?window=<id>&depth=<n>&max=<n>&visibleOnly=<bool>",
                    "The Colibri GUI window/widget hierarchy. Coordinates normalised 0-1 (same space as /api/input/mouse).");
                addEndpoint("/api/gui/labels?visibleOnly=<bool>", "Flat list of all text-bearing widgets with their text and position.");
                addEndpoint("/api/gui/at?x=<f>&y=<f>", "Which widgets contain a normalised (0-1) point, outermost first.");
                addEndpoint("/api/gui/widget/<id>", "Single GUI widget deep dive: geometry, state, text, parent/child ids.");
#ifdef SCRIPT_PROFILER
                addEndpoint("/api/profiler?sort=exclusive|inclusive|calls|avg&max=<n>&minCalls=<n>&include=edges,lines,frames",
                    "Squirrel profile: per function call counts and inclusive/exclusive time. Needs --profileScripts. Bulk sections are opt-in via include.");
                addEndpoint("/api/profiler/function/<id>",
                    "One function in full: its callers, its callees and its hottest lines. Ids come from /api/profiler.");
                addEndpoint("/api/profiler/frames?max=<n>&worst=<bool>",
                    "Per frame script time and call count, with the most expensive root call in each frame. worst=true orders by script time.");
                addEndpoint("/api/profiler/lines?max=<n>",
                    "Hottest source lines by self time, across all functions.");
                addEndpoint("POST /api/profiler/start | /api/profiler/stop | /api/profiler/reset",
                    "Control collection. reset clears the counters but keeps function ids, which is how you skip engine startup and profile only what happens next.");
                addEndpoint("POST /api/profiler/dump?path=<file>&format=json|text",
                    "Write the full uncapped report to disk, so bulk data never enters the response.");
#endif
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

        //Shared helper: capture the next rendered frame at analysis resolution.
        //Like /api/render/frame this runs on the HTTP thread (the capture itself is
        //serviced mid-frame by the Ogre frame listener).
        auto captureAnalysis = [this](httplib::Response& res, CapturedFrame& out) -> bool {
            CapturedFrame full;
            std::string error;
            if(!mFrameCapture->requestCapture(full, error, REQUEST_TIMEOUT_MS)){
                res.status = 503;
                res.set_content(DebugJsonUtil::errorBody("capture failed: " + error), "application/json");
                return false;
            }
            out = RenderInspector::toAnalysisFrame(full);
            return true;
        };

        //GET /api/render/hash — cheap perceptual hash of the live frame, for polling.
        mServer->Get("/api/render/hash", [captureAnalysis](const httplib::Request&, httplib::Response& res){
            CapturedFrame analysis;
            if(!captureAnalysis(res, analysis)) return;

            rapidjson::Document doc;
            RenderInspector::writeHash(doc, analysis);
            res.set_content(DebugJsonUtil::toString(doc), "application/json");
        });

        //POST /api/render/snapshot?name=<slot> — capture now and store for later comparison.
        mServer->Post("/api/render/snapshot", [this, captureAnalysis](const httplib::Request& req, httplib::Response& res){
            const std::string name = req.has_param("name") ? req.get_param_value("name") : "";
            if(name.empty()){
                res.status = 400;
                res.set_content(DebugJsonUtil::errorBody("a \"name\" query parameter is required"), "application/json");
                return;
            }

            CapturedFrame analysis;
            if(!captureAnalysis(res, analysis)) return;

            mFrameStore->save(name, analysis);

            rapidjson::Document doc;
            RenderInspector::writeSnapshot(doc, name, analysis);
            res.set_content(DebugJsonUtil::toString(doc), "application/json");
        });

        //GET /api/render/snapshots — what slots exist.
        mServer->Get("/api/render/snapshots", [this](const httplib::Request&, httplib::Response& res){
            rapidjson::Document doc;
            rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
            doc.SetObject();
            rapidjson::Value arr(rapidjson::kArrayType);
            for(const std::string& name : mFrameStore->names()){
                arr.PushBack(rapidjson::Value(name.c_str(), allocator), allocator);
            }
            doc.AddMember("snapshots", arr, allocator);
            res.set_content(DebugJsonUtil::toString(doc), "application/json");
        });

        //GET /api/render/compare?a=<slot>&b=<slot|live>&w=&h=&threshold=
        mServer->Get("/api/render/compare", [this, captureAnalysis](const httplib::Request& req, httplib::Response& res){
            const std::string aName = req.has_param("a") ? req.get_param_value("a") : "";
            const std::string bName = req.has_param("b") ? req.get_param_value("b") : "live";
            if(aName.empty()){
                res.status = 400;
                res.set_content(DebugJsonUtil::errorBody("an \"a\" snapshot name is required"), "application/json");
                return;
            }

            CapturedFrame frameA;
            if(!mFrameStore->get(aName, frameA)){
                res.status = 404;
                res.set_content(DebugJsonUtil::errorBody("no snapshot named '" + aName + "'"), "application/json");
                return;
            }

            CapturedFrame frameB;
            if(bName == "live"){
                if(!captureAnalysis(res, frameB)) return;
            }else if(!mFrameStore->get(bName, frameB)){
                res.status = 404;
                res.set_content(DebugJsonUtil::errorBody("no snapshot named '" + bName + "'"), "application/json");
                return;
            }

            int gridW = req.has_param("w") ? std::atoi(req.get_param_value("w").c_str()) : 32;
            int gridH = req.has_param("h") ? std::atoi(req.get_param_value("h").c_str()) : 18;
            gridW = std::max(1, std::min(gridW, RenderInspector::MAX_CELLS_X));
            gridH = std::max(1, std::min(gridH, RenderInspector::MAX_CELLS_Y));
            //A fairly sensitive default: unchanged frames diff to exactly 0, so the main
            //risk is missing subtle changes rather than false positives. Small UI changes
            //still want a finer grid (w/h) as well as a lower threshold.
            float threshold = req.has_param("threshold")
                ? static_cast<float>(std::atof(req.get_param_value("threshold").c_str())) : 0.02f;
            if(threshold < 0.0f) threshold = 0.0f;

            rapidjson::Document doc;
            RenderInspector::writeCompare(doc, frameA, frameB, gridW, gridH, threshold);
            res.set_content(DebugJsonUtil::toString(doc), "application/json");
        });

        //GET /api/render/find?rgb=ff0000&tolerance=40&region=x,y,w,h
        mServer->Get("/api/render/find", [captureAnalysis](const httplib::Request& req, httplib::Response& res){
            if(!req.has_param("rgb")){
                res.status = 400;
                res.set_content(DebugJsonUtil::errorBody("an \"rgb\" hex colour is required, e.g. rgb=ff0000"), "application/json");
                return;
            }
            std::string hex = req.get_param_value("rgb");
            if(!hex.empty() && hex[0] == '#') hex.erase(0, 1);
            if(hex.size() != 6 || hex.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos){
                res.status = 400;
                res.set_content(DebugJsonUtil::errorBody("rgb must be 6 hex digits, e.g. rgb=ff0000"), "application/json");
                return;
            }
            const unsigned long packed = std::strtoul(hex.c_str(), nullptr, 16);
            const uint8_t r = static_cast<uint8_t>((packed >> 16) & 0xFF);
            const uint8_t g = static_cast<uint8_t>((packed >> 8) & 0xFF);
            const uint8_t b = static_cast<uint8_t>(packed & 0xFF);

            int tolerance = req.has_param("tolerance") ? std::atoi(req.get_param_value("tolerance").c_str()) : 40;
            tolerance = std::max(0, std::min(tolerance, 255));

            CapturedFrame analysis;
            if(!captureAnalysis(res, analysis)) return;

            //Optional crop before searching, so coordinates stay relative to the crop.
            if(req.has_param("region")){
                float x, y, w, h;
                if(sscanf(req.get_param_value("region").c_str(), "%f,%f,%f,%f", &x, &y, &w, &h) == 4){
                    CapturedFrame cropped = ImageOps::crop(analysis, x, y, w, h);
                    if(!cropped.valid()){
                        res.status = 400;
                        res.set_content(DebugJsonUtil::errorBody("region degenerates to zero pixels"), "application/json");
                        return;
                    }
                    cropped.frameNumber = analysis.frameNumber;
                    analysis = cropped;
                }else{
                    res.status = 400;
                    res.set_content(DebugJsonUtil::errorBody("region must be four comma-separated floats: x,y,w,h"), "application/json");
                    return;
                }
            }

            rapidjson::Document doc;
            RenderInspector::writeFind(doc, analysis, r, g, b, tolerance);
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

        //GET /api/input/actions — discovery.
        mServer->Get("/api/input/actions", [runQuery](const httplib::Request&, httplib::Response& res){
            runQuery(res, [](rapidjson::Document& doc, int& status){
                InputInspector::writeActions(doc, status);
            });
        });

        //GET /api/input/state — what is currently spoofed.
        mServer->Get("/api/input/state", [this, runQuery](const httplib::Request&, httplib::Response& res){
            runQuery(res, [this](rapidjson::Document& doc, int&){
                InputInspector::writeState(doc, *mInputPlayback);
            });
        });

        //Shared helper: run an input mutation on the main thread and write its result.
        auto runInput = [this](httplib::Response& res, std::function<InputPlayback::Result(InputPlayback&)> action){
            auto result = std::make_shared<QueryResult>();
            const bool serviced = mQueue.execute([this, result, action]{
                const InputPlayback::Result r = action(*mInputPlayback);
                rapidjson::Document::AllocatorType& allocator = result->doc.GetAllocator();
                result->doc.SetObject();
                result->doc.AddMember("ok", r.ok, allocator);
                if(!r.ok){
                    result->status = 400;
                    result->doc.AddMember("error", rapidjson::Value(r.error.c_str(), allocator), allocator);
                }else{
                    result->doc.AddMember("frame", mInputPlayback->getFrameNumber(), allocator);
                    if(r.releasesAtFrame >= 0) result->doc.AddMember("releasesAtFrame", r.releasesAtFrame, allocator);
                }
            }, REQUEST_TIMEOUT_MS);

            if(!serviced){
                res.status = 503;
                res.set_content(DebugJsonUtil::errorBody("engine did not service the request (paused or shutting down)"), "application/json");
                return;
            }
            res.status = result->status;
            res.set_content(DebugJsonUtil::toString(result->doc), "application/json");
        };

        //POST /api/input/action {"action","type":"button|axis","value","x","y","frames"}
        mServer->Post("/api/input/action", [this, runInput](const httplib::Request& req, httplib::Response& res){
            rapidjson::Document body;
            body.Parse(req.body.c_str());
            if(body.HasParseError() || !body.IsObject() || !body.HasMember("action") || !body["action"].IsString()){
                res.status = 400;
                res.set_content(DebugJsonUtil::errorBody("body must be JSON with an \"action\" string"), "application/json");
                return;
            }
            const std::string action = body["action"].GetString();
            const std::string type = (body.HasMember("type") && body["type"].IsString()) ? body["type"].GetString() : "button";
            const int frames = (body.HasMember("frames") && body["frames"].IsInt()) ? body["frames"].GetInt() : -1;

            if(type == "button"){
                const bool value = (body.HasMember("value") && body["value"].IsBool()) ? body["value"].GetBool() : true;
                runInput(res, [action, value, frames](InputPlayback& p){ return p.applyButtonAction(action, value, frames); });
            }else if(type == "axis"){
                const float x = (body.HasMember("x") && body["x"].IsNumber()) ? body["x"].GetFloat() : 0.0f;
                const float y = (body.HasMember("y") && body["y"].IsNumber()) ? body["y"].GetFloat() : 0.0f;
                runInput(res, [action, x, y, frames](InputPlayback& p){ return p.applyAxisAction(action, x, y, frames); });
            }else{
                res.status = 400;
                res.set_content(DebugJsonUtil::errorBody("type must be \"button\" or \"axis\""), "application/json");
            }
        });

        //POST /api/input/mouse {"button","pressed","frames"} | {"moveTo":[x,y]}
        mServer->Post("/api/input/mouse", [this, runInput](const httplib::Request& req, httplib::Response& res){
            rapidjson::Document body;
            body.Parse(req.body.c_str());
            if(body.HasParseError() || !body.IsObject()){
                res.status = 400;
                res.set_content(DebugJsonUtil::errorBody("body must be a JSON object"), "application/json");
                return;
            }
            if(body.HasMember("moveTo") && body["moveTo"].IsArray() && body["moveTo"].Size() == 2){
                const float x = body["moveTo"][0].GetFloat();
                const float y = body["moveTo"][1].GetFloat();
                runInput(res, [x, y](InputPlayback& p){ return p.applyMouseMove(x, y); });
            }else if(body.HasMember("button") && body["button"].IsInt()){
                const int button = body["button"].GetInt();
                const bool pressed = (body.HasMember("pressed") && body["pressed"].IsBool()) ? body["pressed"].GetBool() : true;
                const int frames = (body.HasMember("frames") && body["frames"].IsInt()) ? body["frames"].GetInt() : -1;
                runInput(res, [button, pressed, frames](InputPlayback& p){ return p.applyMouseButton(button, pressed, frames); });
            }else{
                res.status = 400;
                res.set_content(DebugJsonUtil::errorBody("provide \"moveTo\":[x,y] or \"button\":<0-2>"), "application/json");
            }
        });

        //POST /api/input/clear — release everything spoofed.
        mServer->Post("/api/input/clear", [this, runInput](const httplib::Request&, httplib::Response& res){
            runInput(res, [](InputPlayback& p){ p.clear(); InputPlayback::Result r; r.ok = true; return r; });
        });

        //GET /api/gui/tree?window=&depth=&max=&visibleOnly=
        mServer->Get("/api/gui/tree", [runQuery](const httplib::Request& req, httplib::Response& res){
            const std::string window = req.has_param("window") ? req.get_param_value("window") : "";
            int depth = req.has_param("depth") ? std::atoi(req.get_param_value("depth").c_str()) : 4;
            int maxNodes = req.has_param("max") ? std::atoi(req.get_param_value("max").c_str()) : 400;
            const bool visibleOnly = req.has_param("visibleOnly") && req.get_param_value("visibleOnly") == "true";
            if(depth < 0) depth = 0;
            if(maxNodes < 1) maxNodes = 1;
            runQuery(res, [window, depth, maxNodes, visibleOnly](rapidjson::Document& doc, int& status){
                GuiInspector::writeTree(doc, status, window, depth, maxNodes, visibleOnly);
            });
        });

        //GET /api/gui/labels?visibleOnly=
        mServer->Get("/api/gui/labels", [runQuery](const httplib::Request& req, httplib::Response& res){
            //Labels default to visible-only; pass visibleOnly=false for everything.
            const bool visibleOnly = !req.has_param("visibleOnly") || req.get_param_value("visibleOnly") != "false";
            runQuery(res, [visibleOnly](rapidjson::Document& doc, int&){
                GuiInspector::writeLabels(doc, visibleOnly);
            });
        });

        //GET /api/gui/at?x=&y=
        mServer->Get("/api/gui/at", [runQuery](const httplib::Request& req, httplib::Response& res){
            if(!req.has_param("x") || !req.has_param("y")){
                res.status = 400;
                res.set_content(DebugJsonUtil::errorBody("x and y (normalised 0-1) are required"), "application/json");
                return;
            }
            const float x = static_cast<float>(std::atof(req.get_param_value("x").c_str()));
            const float y = static_cast<float>(std::atof(req.get_param_value("y").c_str()));
            runQuery(res, [x, y](rapidjson::Document& doc, int& status){
                GuiInspector::writeHitTest(doc, status, x, y);
            });
        });

        //GET /api/gui/widget/<id>
        mServer->Get(R"(/api/gui/widget/(.+))", [runQuery](const httplib::Request& req, httplib::Response& res){
            std::string id = req.matches[1];
            runQuery(res, [id](rapidjson::Document& doc, int& status){
                GuiInspector::writeWidget(doc, status, id);
            });
        });

#ifdef SCRIPT_PROFILER
        //The profiler's data is written from inside the vm's debug hook, so like the
        //inspection endpoints every one of these reads it through the main thread queue.

        //GET /api/profiler?sort=&max=&minCalls=&include=edges,lines,frames
        mServer->Get("/api/profiler", [runQuery](const httplib::Request& req, httplib::Response& res){
            ProfileQuery query;
            if(req.has_param("sort")){
                const std::string sort = req.get_param_value("sort");
                if(!ProfileQuery::parseSort(sort, query.sort)){
                    res.status = 400;
                    res.set_content(DebugJsonUtil::errorBody(
                        "sort must be one of exclusive, inclusive, calls, avg"), "application/json");
                    return;
                }
            }
            if(req.has_param("max")) query.maxFunctions = static_cast<uint32_t>(std::max(0, std::atoi(req.get_param_value("max").c_str())));
            if(req.has_param("minCalls")) query.minCalls = static_cast<uint64_t>(std::max(0, std::atoi(req.get_param_value("minCalls").c_str())));

            //Bulk sections stay off unless asked for, so the default response is small.
            if(req.has_param("include")){
                const std::string include = req.get_param_value("include");
                query.includeEdges = include.find("edges") != std::string::npos;
                query.includeLines = include.find("lines") != std::string::npos;
                query.includeFrames = include.find("frames") != std::string::npos;
            }

            runQuery(res, [query](rapidjson::Document& doc, int& status){
                ProfilerInspector::writeProfile(doc, status, query);
            });
        });

        //GET /api/profiler/function/<id>
        mServer->Get(R"(/api/profiler/function/(\d+))", [runQuery](const httplib::Request& req, httplib::Response& res){
            const uint32_t id = static_cast<uint32_t>(std::strtoul(std::string(req.matches[1]).c_str(), nullptr, 10));
            runQuery(res, [id](rapidjson::Document& doc, int& status){
                ProfilerInspector::writeFunctionDetail(doc, status, id);
            });
        });

        //GET /api/profiler/frames?max=<n>&worst=<bool>
        mServer->Get("/api/profiler/frames", [runQuery](const httplib::Request& req, httplib::Response& res){
            ProfileQuery query;
            //Enough of a function table to resolve the ids the frames refer to, without
            //burying the timeline that was actually asked for.
            query.maxFunctions = 10;
            query.includeFrames = true;
            if(req.has_param("max")) query.maxFrames = static_cast<uint32_t>(std::max(0, std::atoi(req.get_param_value("max").c_str())));
            if(req.has_param("worst")){
                const std::string worst = req.get_param_value("worst");
                query.worstFramesFirst = (worst == "true" || worst == "1");
            }

            runQuery(res, [query](rapidjson::Document& doc, int& status){
                ProfilerInspector::writeProfile(doc, status, query);
            });
        });

        //GET /api/profiler/lines?max=<n>
        mServer->Get("/api/profiler/lines", [runQuery](const httplib::Request& req, httplib::Response& res){
            ProfileQuery query;
            //Line entries carry their own function name, so the table is only context.
            query.maxFunctions = 10;
            query.includeLines = true;
            if(req.has_param("max")) query.maxLines = static_cast<uint32_t>(std::max(0, std::atoi(req.get_param_value("max").c_str())));

            runQuery(res, [query](rapidjson::Document& doc, int& status){
                ProfilerInspector::writeProfile(doc, status, query);
            });
        });

        //POST /api/profiler/start | /stop | /reset
        auto profilerControl = [runQuery](httplib::Response& res, ProfilerInspector::Control control){
            runQuery(res, [control](rapidjson::Document& doc, int& status){
                ProfilerInspector::writeControl(doc, status, control);
            });
        };
        mServer->Post("/api/profiler/start", [profilerControl](const httplib::Request&, httplib::Response& res){
            profilerControl(res, ProfilerInspector::Control::START);
        });
        mServer->Post("/api/profiler/stop", [profilerControl](const httplib::Request&, httplib::Response& res){
            profilerControl(res, ProfilerInspector::Control::STOP);
        });
        mServer->Post("/api/profiler/reset", [profilerControl](const httplib::Request&, httplib::Response& res){
            profilerControl(res, ProfilerInspector::Control::RESET);
        });

        //POST /api/profiler/dump?path=<file>&format=json|text
        mServer->Post("/api/profiler/dump", [runQuery](const httplib::Request& req, httplib::Response& res){
            if(!req.has_param("path")){
                res.status = 400;
                res.set_content(DebugJsonUtil::errorBody("a \"path\" to write to is required"), "application/json");
                return;
            }
            const std::string path = req.get_param_value("path");
            //Otherwise infer it from the extension, the same as the --profileScripts flag.
            const bool json = req.has_param("format")
                ? req.get_param_value("format") == "json"
                : ScriptProfiler::pathWantsJson(path);

            runQuery(res, [path, json](rapidjson::Document& doc, int& status){
                ProfilerInspector::writeDump(doc, status, path, json);
            });
        });
#endif
    }
}

#endif
