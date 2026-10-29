#pragma once

#include "Threading/JobDispatcher.h"
#include "Threading/Jobs/Job.h"
#include "System/SystemSetup/SystemSettings.h"
#include <condition_variable>
#include <memory>
#include <mutex>

namespace AV{
    class AudioPoolGate{
        struct State{
            std::mutex mutex;
            std::condition_variable changed;
            unsigned entered = 0;
            bool released = false;
        };
        class GateJob : public Job{
        public:
            explicit GateJob(std::shared_ptr<State> state) : mState(std::move(state)) {}
            void process() override{
                std::unique_lock<std::mutex> lock(mState->mutex);
                ++mState->entered;
                mState->changed.notify_all();
                mState->changed.wait(lock, [this]{ return mState->released; });
            }
            void finish() override {}
        private:
            std::shared_ptr<State> mState;
        };
        std::shared_ptr<State> mState = std::make_shared<State>();
    public:
        AudioPoolGate(){
            for(unsigned i = 0; i < SystemSettings::getNumWorkerThreads(); ++i)
                JobDispatcher::dispatchJob(new GateJob(mState));
        }
        ~AudioPoolGate(){ release(); }
        bool wait(){
            std::unique_lock<std::mutex> lock(mState->mutex);
            return mState->changed.wait_for(lock, std::chrono::seconds(1), [this]{
                return mState->entered == SystemSettings::getNumWorkerThreads();
            });
        }
        void release(){
            { std::lock_guard<std::mutex> lock(mState->mutex); mState->released = true; }
            mState->changed.notify_all();
        }
    };
}
