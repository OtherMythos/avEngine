#pragma once

#include "PhysicsWorld.h"

//TODO I'd rather not have to enclude the entire shape manager for the sake of a single pointer.
//I feel at some point the shape ptr should be taken out of there and put somewhere else.
#include "World/Physics/PhysicsShapeManager.h"
#include "btBulletDynamicsCommon.h"

#include <mutex>

namespace AV{
    class DynamicsWorldThreadLogic;

    class DynamicsWorld : public PhysicsWorld{
    public:
        DynamicsWorld();
        ~DynamicsWorld();

        void setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic);
        void addBody(btRigidBody* body);

        btRigidBody* createRigidBody(const btRigidBody::btRigidBodyConstructionInfo& info);

        void update();

    private:
        std::mutex dynWorldMutex;

        DynamicsWorldThreadLogic* mDynLogic;
    };
}
