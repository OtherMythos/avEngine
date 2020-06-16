#pragma once

#include "PhysicsWorldThreadLogic.h"
#include "System/EnginePrerequisites.h"

class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btCollisionObject;

namespace AV{
    class CollisionWorldThreadLogic : public PhysicsWorldThreadLogic{
    public:
        CollisionWorldThreadLogic(uint8 worldId);
        ~CollisionWorldThreadLogic();

        void updateWorld();

        std::mutex objectInputBufferMutex;

    public:
        enum class ObjectCommandType{
            COMMAND_TYPE_NONE,

            COMMAND_TYPE_ADD_OBJECT,
            COMMAND_TYPE_REMOVE_OBJECT,
            COMMAND_TYPE_DESTROY_OBJECT,
        };

        struct ObjectCommandBufferEntry{
            ObjectCommandType type;
            btCollisionObject* body;
        };

        std::vector<ObjectCommandBufferEntry> inputObjectCommandBuffer;

    protected:
        void constructWorld();
        void destroyWorld();

        void checkInputBuffers();
        void updateOutputBuffer();

        void _processObjectInputBuffer();

    private:
        btBroadphaseInterface* mBroadphaseCollision;
        btDefaultCollisionConfiguration* mCollisionWorldConfiguration;
        btCollisionDispatcher* mCollisionDispatcher;

        uint8 mWorldId;

    public:
        uint8 getWorldId() const { return mWorldId; }
    };
}
