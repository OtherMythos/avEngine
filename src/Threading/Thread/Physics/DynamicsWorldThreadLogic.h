#pragma once

#include <mutex>
#include <vector>

#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btRigidBody;

namespace AV{
    class PhysicsThread;

    class DynamicsWorldThreadLogic{
        //friend class PhysicsThread;

    public:
        DynamicsWorldThreadLogic();

        enum class InputBufferCommandType{
            COMMAND_TYPE_NONE,
            COMMAND_TYPE_SET_POSITION,
        };

        struct inputBufferEntry{
            InputBufferCommandType type;
            btRigidBody* body;
            btVector3 val;
        };

        enum class ObjectCommandType{
            COMMAND_TYPE_NONE,

            COMMAND_TYPE_ADD,
            COMMAND_TYPE_REMOVE,
            COMMAND_TYPE_DESTROY,

            COMMAND_TYPE_ADD_CHUNK,
            COMMAND_TYPE_REMOVE_CHUNK
        };

        struct objectCommandBufferEntry{
            ObjectCommandType type;
            btRigidBody* body;
        };

        struct outputBufferEntry{
            btRigidBody* body;
            btVector3 pos;
            btQuaternion orientation;
        };

        /**
        Notify the dynamics world that a rigid body has moved.
        This is intended to be called by the DynamicsWorldMotionState.
        */
        void _notifyBodyMoved(btRigidBody *body);

        void checkWorldConstructDestruct(bool worldShouldExist);

        /**
        Step the dynamics world.
        */
        void updateWorld();

        //Only the DynamicsWorld class should have a pointer to this anyway.
        //TODO make this nicer, as in make it so only the dynamics world has access to it.
        std::mutex objectInputBufferMutex;
        std::mutex inputBufferMutex;
        std::mutex outputBufferMutex;

        btVector3 worldOriginChangeOffset;
        bool worldShifted = false;

        std::vector<inputBufferEntry> inputBuffer;
        //A separate buffer to deal with more complex requests (add to world, remove from world, destroy shape).
        std::vector<objectCommandBufferEntry> inputObjectCommandBuffer;

        std::vector<outputBufferEntry> outputBuffer;

    private:
        void constructWorld();
        void destroyWorld();

        void _processInputBuffer();
        void _processObjectInputBuffer();

        void _performOriginShift(btVector3 offset);

        void checkInputBuffers();
        void updateOutputBuffer();

        //The bodies that have moved this frame need to be kept track of.
        std::vector<btRigidBody*> mMovedBodies;

    private:
        btDefaultCollisionConfiguration* mCollisionConfiguration;
        btCollisionDispatcher* mDispatcher;
        btBroadphaseInterface* mOverlappingPairCache;
        btSequentialImpulseConstraintSolver* mSolver;
        btDiscreteDynamicsWorld* mDynamicsWorld = 0;

    };
}
