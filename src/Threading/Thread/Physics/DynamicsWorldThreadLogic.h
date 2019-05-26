#pragma once

#include <mutex>
#include <vector>

#include <LinearMath/btVector3.h>

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btRigidBody;

namespace AV{
    class PhysicsThread;
    
    class DynamicsWorldThreadLogic{
        friend class PhysicsThread;
        
    public:
        //Just creation logic for now.
        struct inputBufferEntry{
            btRigidBody* body;
        };
        
        struct outputBufferEntry{
            btRigidBody* body;
            btVector3 pos;
        };
        
        //Only the DynamicsWorld class should have a pointer to this anyway.
        //TODO make this nicer
        std::mutex inputBufferMutex;
        std::mutex outputBufferMutex;
        
        std::vector<inputBufferEntry> inputBuffer;
        
        std::vector<outputBufferEntry> outputBuffer;
        
        std::vector<btRigidBody*> entities;
        
    private:
        DynamicsWorldThreadLogic();
        
        void constructWorld();
        void destroyWorld();
        
        /**
        Step the dynamics world.
        */
        void updateWorld();
        
        void checkInputBuffer();
        void updateOutputBuffer();
        
        void checkWorldConstructDestruct(bool worldShouldExist);
        
    private:
        btDefaultCollisionConfiguration* mCollisionConfiguration;
        btCollisionDispatcher* mDispatcher;
        btBroadphaseInterface* mOverlappingPairCache;
        btSequentialImpulseConstraintSolver* mSolver;
        btDiscreteDynamicsWorld* mDynamicsWorld;        
        
    };
}
