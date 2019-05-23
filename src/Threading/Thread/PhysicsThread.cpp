#include "PhysicsThread.h"

#include "Logger/Log.h"

namespace AV{
    PhysicsThread::PhysicsThread(){
        
    }
    
    void PhysicsThread::run(){
        AV_INFO("Wow physics thread");
        mRunning = true;
        
        /*while(mRunning){
            
        }*/
        
    }
}
