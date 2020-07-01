#pragma once

#include "PhysicsWorldThreadLogic.h"
#include "System/EnginePrerequisites.h"
#include "World/Physics/Worlds/CollisionWorldUtils.h"

class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btCollisionObject;
class btPersistentManifold;
class btManifoldPoint;

namespace AV{
    class CollisionWorldThreadLogic : public PhysicsWorldThreadLogic{
    public:
        CollisionWorldThreadLogic(uint8 worldId);
        ~CollisionWorldThreadLogic();

        void updateWorld();

        std::mutex objectInputBufferMutex;
        std::mutex inputBufferMutex;
        std::mutex objectOutputBufferMutex;

    public:
        enum class ObjectCommandType{
            COMMAND_TYPE_NONE,

            COMMAND_TYPE_ADD_OBJECT,
            COMMAND_TYPE_REMOVE_OBJECT,
            COMMAND_TYPE_DESTROY_OBJECT,
        };

        enum class InputBufferType{
            COMMAND_TYPE_NONE,

            COMMAND_TYPE_SET_POSITION
        };

        struct ObjectCommandBufferEntry{
            ObjectCommandType type;
            btCollisionObject* object;
        };

        struct InputBufferEntry{
            InputBufferType type;
            btCollisionObject* object;
            btVector3 val;
        };

        struct ObjectEventBufferEntry{
            const btCollisionObject* sender;
            const btCollisionObject* receiver;
            CollisionObjectEventMask::CollisionObjectEventMask eventType;
        };

        //Shared vectors.
        //Object commands
        std::vector<ObjectCommandBufferEntry> inputObjectCommandBuffer;
        std::vector<ObjectEventBufferEntry> outputObjectEventBuffer;
        //input commands, stuff like set position.
        std::vector<InputBufferEntry> inputCommandBuffer;

    protected:
        void constructWorld();
        void destroyWorld();

        void checkInputBuffers();
        void updateOutputBuffer();

        void _processObjectInputBuffer();
        void _processInputBuffer();

        //Events write into this and then they're eventually moved into the main output buffer.
        //TODO an optimisation would be having two buffers which are swapped between the two threads. This is more complicated however so its like this for now.
        std::vector<ObjectEventBufferEntry> tempObjectEventBuffer;

    private:
        btBroadphaseInterface* mBroadphaseCollision;
        btDefaultCollisionConfiguration* mCollisionWorldConfiguration;
        btCollisionDispatcher* mCollisionDispatcher;

        static void contactStartedCallback(btPersistentManifold* const& manifold);
        static void contactEndedCallback(btPersistentManifold* const& manifold);
        static bool contactProcessedCallback(btManifoldPoint& cp, void* body0, void* body1);
        static void _processStartedEndedCallback(btPersistentManifold* const& manifold, bool started);

        uint8 mWorldId;

    public:
        uint8 getWorldId() const { return mWorldId; }
    };
}
