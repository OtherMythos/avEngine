#pragma once

namespace AV{
    class Job{
    public:
        virtual void process() = 0;
        virtual void finish() = 0;
        
        virtual ~Job() {}
    };
}
