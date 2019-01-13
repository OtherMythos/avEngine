#pragma once

#include "Job.h"
#include <chrono>

namespace AV{
    class TestJob : public Job{
        void process(){
            AV_INFO("Processing");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        void finish(){
            AV_INFO("Finished");
        }
    };
}
