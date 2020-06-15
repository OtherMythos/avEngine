#pragma once

#include "PhysicsWorldThreadLogic.h"

class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;

namespace AV{
    class CollisionWorldThreadLogic : public PhysicsWorldThreadLogic{
    public:
        CollisionWorldThreadLogic();
        ~CollisionWorldThreadLogic();

        void updateWorld();

    protected:
        void constructWorld();
        void destroyWorld();

    private:
        btBroadphaseInterface* mBroadphaseCollision;
        btDefaultCollisionConfiguration* mCollisionWorldConfiguration;
        btCollisionDispatcher* mCollisionDispatcher;
    };
}
