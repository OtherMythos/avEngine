#pragma once

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

namespace AV{
    class PhysicsThread;
    
    class DynamicsWorldThreadLogic{
        friend class PhysicsThread;
    private:
        DynamicsWorldThreadLogic();
        
        void constructWorld();
        void destroyWorld();
        
        /**
        Step the dynamics world.
        */
        void updateWorld();
        
        void checkWorldConstructDestruct(bool worldShouldExist);
        
    private:
        btDefaultCollisionConfiguration* mCollisionConfiguration;
        btCollisionDispatcher* mDispatcher;
        btBroadphaseInterface* mOverlappingPairCache;
        btSequentialImpulseConstraintSolver* mSolver;
        btDiscreteDynamicsWorld* mDynamicsWorld;
    };
}
