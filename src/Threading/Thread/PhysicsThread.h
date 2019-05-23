#pragma once

namespace AV{
    class PhysicsThread{
    public:
        PhysicsThread();
        
        void run();
        
    private:
        bool mRunning = false;
    };
}
