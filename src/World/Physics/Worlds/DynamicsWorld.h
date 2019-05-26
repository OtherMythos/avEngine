#pragma once

#include "PhysicsWorld.h"

#include <mutex>

class btRigidBody;

namespace AV{
    class DynamicsWorldThreadLogic;
    
    class DynamicsWorld : public PhysicsWorld{
    public:
        DynamicsWorld();
        ~DynamicsWorld();
        
        void setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic);
        void addBody(btRigidBody* body);
        
        void update();
        
    private:
        std::mutex dynWorldMutex;
        
        DynamicsWorldThreadLogic* mDynLogic;
        
        //TODO make sure this isn't there
        btRigidBody *tmpBody;
    };
}
