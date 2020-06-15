#pragma once

#include "PhysicsWorldThreadLogic.h"

class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btCollisionObject;

namespace AV{
    class CollisionWorldThreadLogic : public PhysicsWorldThreadLogic{
    public:
        CollisionWorldThreadLogic();
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

    private:
        btBroadphaseInterface* mBroadphaseCollision;
        btDefaultCollisionConfiguration* mCollisionWorldConfiguration;
        btCollisionDispatcher* mCollisionDispatcher;
    };
}
