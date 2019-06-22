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

        enum class ObjectCommandType{
            COMMAND_TYPE_NONE,
            COMMAND_TYPE_ADD,
            COMMAND_TYPE_REMOVE,
            COMMAND_TYPE_DESTROY
        };

        struct objectCommandBufferEntry{
            ObjectCommandType type;
            btRigidBody* body;
        };

        struct outputBufferEntry{
            btRigidBody* body;
            btVector3 pos;
        };

        /**
        Notify the dynamics world that a rigid body has moved.
        This is intended to be called by the DynamicsWorldMotionState.
        */
        void _notifyBodyMoved(btRigidBody *body);

        //Only the DynamicsWorld class should have a pointer to this anyway.
        //TODO make this nicer, as in make it so only the dynamics world has access to it.
        std::mutex inputBufferMutex;
        std::mutex outputBufferMutex;

        std::vector<inputBufferEntry> inputBuffer;
        //A separate buffer to deal with more complex requests (add to world, remove from world, destroy shape).
        std::vector<objectCommandBufferEntry> inputObjectCommandBuffer;

        std::vector<outputBufferEntry> outputBuffer;

    private:
        DynamicsWorldThreadLogic();

        void constructWorld();
        void destroyWorld();

        /**
        Step the dynamics world.
        */
        void updateWorld();

        void _processInputBuffer();
        void _processObjectInputBuffer();

        void checkInputBuffers();
        void updateOutputBuffer();

        //The bodies that have moved this frame need to be kept track of.
        std::vector<btRigidBody*> mMovedBodies;

        void checkWorldConstructDestruct(bool worldShouldExist);

    private:
        btDefaultCollisionConfiguration* mCollisionConfiguration;
        btCollisionDispatcher* mDispatcher;
        btBroadphaseInterface* mOverlappingPairCache;
        btSequentialImpulseConstraintSolver* mSolver;
        btDiscreteDynamicsWorld* mDynamicsWorld = 0;

    };
}
