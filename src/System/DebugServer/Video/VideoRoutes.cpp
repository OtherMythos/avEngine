#ifdef DEBUG_SERVER
#include "System/DebugServer/DebugServer.h"
#include "System/DebugServer/Video/VideoRecorder.h"
#include "System/DebugServer/Inspection/DebugJsonUtil.h"
#include "System/Base.h"
#include "System/BaseSingleton.h"
#include "httplib.h"

#include <filesystem>

namespace AV{
    namespace{
        std::string videoStatus(const VideoRecorder::Status& status){
            rapidjson::Document doc;
            auto& a = doc.GetAllocator();
            doc.SetObject();
            doc.AddMember("state", rapidjson::Value(status.state.c_str(), a), a);
            doc.AddMember("outputPath", rapidjson::Value(status.outputPath.c_str(), a), a);
            doc.AddMember("lastError", rapidjson::Value(status.lastError.c_str(), a), a);
            doc.AddMember("fps", status.fps, a);
            doc.AddMember("frameLimit", DebugJsonUtil::uint64Value(status.frameLimit), a);
            doc.AddMember("framesCaptured", DebugJsonUtil::uint64Value(status.framesCaptured), a);
            doc.AddMember("framesEncoded", DebugJsonUtil::uint64Value(status.framesEncoded), a);
            doc.AddMember("queuedFrames", static_cast<unsigned int>(status.queued), a);
            return DebugJsonUtil::toString(doc);
        }
    }

    void DebugServer::_registerVideoRoutes(){
        mServer->Get("/api/video/status", [this](const httplib::Request&, httplib::Response& res){
            res.set_content(videoStatus(mVideoRecorder->status()), "application/json");
        });

        mServer->Post("/api/video/start", [this](const httplib::Request& req, httplib::Response& res){
            rapidjson::Document body;
            body.Parse(req.body.c_str());
            if(body.HasParseError() || !body.IsObject()){
                res.status = 400;
                res.set_content(DebugJsonUtil::errorBody("body must be a JSON object"), "application/json");
                return;
            }
            std::string outputDir;
            int fps = 60, width = 0, height = 0;
            uint64 frameLimit = 0;
            for(auto it = body.MemberBegin(); it != body.MemberEnd(); ++it){
                const std::string name = it->name.GetString();
                const auto& value = it->value;
                if(name == "outputDir" && value.IsString() && value.GetStringLength() > 0 && value.GetStringLength() <= 4096){
                    outputDir.assign(value.GetString(), value.GetStringLength());
                }else if(name == "fps" && value.IsInt() && value.GetInt() >= 1 && value.GetInt() <= 240){
                    fps = value.GetInt();
                }else if(name == "frameLimit" && value.IsUint64() && value.GetUint64() > 0){
                    frameLimit = value.GetUint64();
                }else if(name == "width" && value.IsInt() && value.GetInt() > 0){
                    width = value.GetInt();
                }else if(name == "height" && value.IsInt() && value.GetInt() > 0){
                    height = value.GetInt();
                }else{
                    res.status = 400;
                    res.set_content(DebugJsonUtil::errorBody("invalid video parameter"), "application/json");
                    return;
                }
            }
            auto result = std::make_shared<std::pair<int, std::string>>(200, "");
            const bool serviced = mQueue.execute([this, result, outputDir, fps, frameLimit, width, height]{
                try{
                    Base* engine = BaseSingleton::getBase();
                    auto& clock = engine->simulationClock();
                    if(outputDir.empty() || outputDir.find('\0') != std::string::npos ||
                       !std::filesystem::path(outputDir).is_absolute() || clock.rate() % fps != 0 ||
                       (width == 0) != (height == 0) || (width & 1) || (height & 1) ||
                       width > 8192 || height > 8192){
                        result->first = 400;
                        result->second = DebugJsonUtil::errorBody("outputDir must be absolute and new; fps must divide the fixed update rate; dimensions must be an even pair up to 8192");
                        return;
                    }
                    if(!engine->isOpen() || clock.mode() == SimulationClock::Mode::STEPPING){
                        result->first = 409;
                        result->second = DebugJsonUtil::errorBody("start requires an open engine outside a step");
                        return;
                    }
                    std::string error;
                    if(!mVideoRecorder->start(outputDir, fps, frameLimit, width, height, clock.tick(), clock.rate(), error)){
                        result->first = 409;
                        result->second = DebugJsonUtil::errorBody(error);
                        return;
                    }
                    clock.setOfflineCapture(true);
                    result->second = videoStatus(mVideoRecorder->status());
                }catch(const std::exception& e){
                    result->first = 500;
                    result->second = DebugJsonUtil::errorBody(e.what());
                }
            }, REQUEST_TIMEOUT_MS);
            res.status = serviced ? result->first : 503;
            res.set_content(serviced ? result->second : DebugJsonUtil::errorBody("video start was not acknowledged; inspect status before retrying"), "application/json");
        });

        mServer->Post("/api/video/stop", [this](const httplib::Request&, httplib::Response& res){
            auto result = std::make_shared<std::pair<int, std::string>>(200, "");
            const bool serviced = mQueue.execute([this, result]{
                if(mVideoRecorder->status().state == "idle"){
                    result->first = 409;
                    result->second = DebugJsonUtil::errorBody("no recording has started");
                    return;
                }
                auto& clock = BaseSingleton::getBase()->simulationClock();
                if(!mVideoRecorder->atFrameBoundary(clock.tick())){
                    result->first = 409;
                    result->second = DebugJsonUtil::errorBody("step " +
                        std::to_string(mVideoRecorder->ticksUntilFrame(clock.tick())) +
                        " more ticks to finish the current video frame before stopping");
                    return;
                }
                mVideoRecorder->stop();
                clock.setOfflineCapture(false);
                result->second = videoStatus(mVideoRecorder->status());
            }, REQUEST_TIMEOUT_MS);
            res.status = serviced ? result->first : 503;
            res.set_content(serviced ? result->second : DebugJsonUtil::errorBody("video stop was not acknowledged; inspect status before retrying"), "application/json");
        });
    }
}
#endif
