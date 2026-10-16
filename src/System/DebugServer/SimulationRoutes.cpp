#ifdef DEBUG_SERVER
#include "DebugServer.h"
#include "Inspection/DebugJsonUtil.h"
#include "System/Base.h"
#include "System/BaseSingleton.h"
#include "httplib.h"

#include <condition_variable>
#include <mutex>

namespace AV{
    namespace{
        void writeSimulation(rapidjson::Document& doc){
            const auto& clock = BaseSingleton::getBase()->simulationClock();
            auto& a = doc.GetAllocator();
            doc.SetObject();
            const char* mode = clock.mode() == SimulationClock::Mode::RUNNING ? "running" :
                clock.mode() == SimulationClock::Mode::STEPPING ? "stepping" : "paused";
            doc.AddMember("mode", rapidjson::Value(mode, a), a);
            doc.AddMember("tick", DebugJsonUtil::uint64Value(clock.tick()), a);
            doc.AddMember("simulationSeconds", clock.seconds(), a);
            doc.AddMember("fixedUpdateRate", clock.rate(), a);
            doc.AddMember("speed", clock.speed(), a);
            doc.AddMember("pauseReason", rapidjson::Value(clock.pauseReason().c_str(), a), a);
            if(clock.stepId()){
                rapidjson::Value step(rapidjson::kObjectType);
                step.AddMember("id", DebugJsonUtil::uint64Value(clock.stepId()), a);
                step.AddMember("startTick", DebugJsonUtil::uint64Value(clock.stepStartTick()), a);
                step.AddMember("ticksRequested", DebugJsonUtil::uint64Value(clock.stepRequested()), a);
                step.AddMember("ticksExecuted", DebugJsonUtil::uint64Value(clock.stepExecuted()), a);
                step.AddMember("complete", clock.mode() != SimulationClock::Mode::STEPPING, a);
                doc.AddMember("step", step, a);
            }
        }
    }

    struct DebugServer::SimulationStepRequest{
        std::mutex mutex;
        std::condition_variable cv;
        bool done = false;
        int status = 200;
        std::string response;
    };

    void DebugServer::_completeSimulationStep(bool shuttingDown){
        if(!mSimulationStep) return;
        if(!shuttingDown && BaseSingleton::getBase()->simulationClock().mode() == SimulationClock::Mode::STEPPING) return;
        auto pending = std::move(mSimulationStep);
        rapidjson::Document doc;
        writeSimulation(doc);
        {
            std::lock_guard<std::mutex> lock(pending->mutex);
            pending->response = shuttingDown ? DebugJsonUtil::errorBody("engine shut down during step") : DebugJsonUtil::toString(doc);
            pending->status = shuttingDown ? 503 : 200;
            pending->done = true;
        }
        pending->cv.notify_all();
    }

    void DebugServer::_registerSimulationRoutes(){
        auto handle = [this](const httplib::Request& req, httplib::Response& res, const std::string& command){
            rapidjson::Document body;
            body.Parse(req.body.empty() ? "{}" : req.body.c_str());
            auto fail = [&res](int status, const char* error){
                res.status = status;
                res.set_content(DebugJsonUtil::errorBody(error), "application/json");
            };
            if(body.HasParseError() || !body.IsObject()){
                fail(400, "body must be a JSON object");
                return;
            }
            uint64_t ticks = 1;
            double speed = -1;
            std::string reason = "agent pause";
            for(auto it = body.MemberBegin(); it != body.MemberEnd(); ++it){
                const std::string name = it->name.GetString();
                if(command == "step" && name == "ticks" && it->value.IsUint64() &&
                   it->value.GetUint64() >= 1 && it->value.GetUint64() <= 10000){
                    ticks = it->value.GetUint64();
                }else if(command == "resume" && name == "speed" && it->value.IsNumber() &&
                         it->value.GetDouble() > 0 && it->value.GetDouble() <= 1){
                    speed = it->value.GetDouble();
                }else if(command == "pause" && name == "reason" && it->value.IsString() && it->value.GetStringLength() <= 1024){
                    reason.assign(it->value.GetString(), it->value.GetStringLength());
                }else{
                    fail(400, "invalid parameter: step accepts ticks 1..10000; resume accepts speed (0,1]; pause accepts reason up to 1024 bytes");
                    return;
                }
            }

            struct Result{
                int status = 200;
                std::string response;
            };
            auto result = std::make_shared<Result>();
            auto pending = std::make_shared<SimulationStepRequest>();
            const bool serviced = mQueue.execute([this, command, ticks, speed, reason, result, pending]{
                _completeSimulationStep();
                Base* engine = BaseSingleton::getBase();
                auto& clock = engine->simulationClock();
                if(command == "pause"){
                    engine->pauseSimulation(reason);
                    _completeSimulationStep();
                }else if(command == "resume"){
                    if(!engine->resumeSimulation(speed < 0 ? clock.speed() : speed)){
                        result->status = 409;
                        result->response = DebugJsonUtil::errorBody("pause the active step before resuming");
                        return;
                    }
                }else if(command == "step"){
                    if(!engine->isOpen() || !clock.step(ticks)){
                        result->status = 409;
                        result->response = DebugJsonUtil::errorBody("step requires a paused, open engine");
                        return;
                    }
                    mSimulationStep = pending;
                }
                rapidjson::Document doc;
                writeSimulation(doc);
                result->response = DebugJsonUtil::toString(doc);
            }, REQUEST_TIMEOUT_MS);
            if(!serviced){
                fail(503, "simulation request was not acknowledged; inspect GET /api/simulation before retrying a mutation");
                return;
            }
            if(command == "step" && result->status == 200){
                std::unique_lock<std::mutex> lock(pending->mutex);
                if(pending->cv.wait_for(lock, std::chrono::milliseconds(REQUEST_TIMEOUT_MS), [&]{ return pending->done; })){
                    result->status = pending->status;
                    result->response = pending->response;
                }else{
                    //The operation remains accepted. Poll status; repeating POST would request new work.
                    result->status = 202;
                }
            }
            res.status = result->status;
            res.set_content(result->response, "application/json");
        };
        mServer->Get("/api/simulation", [handle](const httplib::Request& req, httplib::Response& res){ handle(req, res, "status"); });
        mServer->Post("/api/simulation/pause", [handle](const httplib::Request& req, httplib::Response& res){ handle(req, res, "pause"); });
        mServer->Post("/api/simulation/resume", [handle](const httplib::Request& req, httplib::Response& res){ handle(req, res, "resume"); });
        mServer->Post("/api/simulation/step", [handle](const httplib::Request& req, httplib::Response& res){ handle(req, res, "step"); });
    }
}
#endif
