#pragma once

#include "PhysicsWorldThreadLogic.h"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btRigidBody;

namespace AV{
    class PhysicsThread;

    class DynamicsWorldThreadLogic : public PhysicsWorldThreadLogic{
        //friend class PhysicsThread;

    public:
        DynamicsWorldThreadLogic();

        enum class InputBufferCommandType{
            COMMAND_TYPE_NONE,
            COMMAND_TYPE_SET_POSITION,
            COMMAND_TYPE_SET_LINEAR_FACTOR,
            COMMAND_TYPE_SET_LINEAR_VELOCITY,
        };

        struct InputBufferEntry{
            InputBufferCommandType type;
            btRigidBody* body;
            btVector3 val;
        };

        enum class ObjectCommandType{
            COMMAND_TYPE_NONE,

            COMMAND_TYPE_ADD_BODY,
            COMMAND_TYPE_REMOVE_BODY,
            COMMAND_TYPE_DESTROY_BODY,

            COMMAND_TYPE_ADD_CHUNK,
            COMMAND_TYPE_REMOVE_CHUNK,

            COMMAND_TYPE_ADD_TERRAIN
        };

        struct ObjectCommandBufferEntry{
            ObjectCommandType type;
            btRigidBody* body;
        };

        struct OutputBufferEntry{
            btRigidBody* body;
            btVector3 pos;
            btQuaternion orientation;
            btVector3 linearVelocity;
        };

        enum class ObjectDestructionType{
            DESTRUCTION_TYPE_BODY,
            DESTRUCTION_TYPE_CHUNK
        };

        struct OutputDestructionBufferEntry{
            btRigidBody* body;
            ObjectDestructionType type;
        };

        /**
        Notify the dynamics world that a rigid body has moved.
        This is intended to be called by the DynamicsWorldMotionState.
        */
        void _notifyBodyMoved(btRigidBody *body);

        /**
        Step the dynamics world.
        */
        void updateWorld();

        //Only the DynamicsWorld class should have a pointer to this anyway.
        //TODO make this nicer, as in make it so only the dynamics world has access to it.
        std::mutex objectInputBufferMutex;
        std::mutex inputBufferMutex;
        std::mutex outputBufferMutex;
        std::mutex outputDestructionBufferMutex;

        btVector3 worldOriginChangeOffset;
        bool worldShifted = false;

        std::vector<InputBufferEntry> inputBuffer;
        //A separate buffer to deal with more complex requests (add to world, remove from world, destroy shape).
        std::vector<ObjectCommandBufferEntry> inputObjectCommandBuffer;

        std::vector<OutputBufferEntry> outputBuffer;
        std::vector<OutputDestructionBufferEntry> outputDestructionBuffer;

    protected:
        void constructWorld();
        void destroyWorld();

    private:

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
        //Inherits mPhysicsWorld
        //btDiscreteDynamicsWorld* mDynamicsWorld = 0;

    };
}
