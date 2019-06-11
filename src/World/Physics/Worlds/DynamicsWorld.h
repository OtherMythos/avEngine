#pragma once

#include "PhysicsWorld.h"

//TODO I'd rather not have to enclude the entire shape manager for the sake of a single pointer.
//I feel at some point the shape ptr should be taken out of there and put somewhere else.
#include "World/Physics/PhysicsShapeManager.h"
#include "btBulletDynamicsCommon.h"

//TODO this is here on a trial basis. If it works out the name and class should be re-named and put somewhere else.
#include "Scripting/ScriptDataPacker.h"

#include <mutex>
#include <memory>

namespace AV{
    class DynamicsWorldThreadLogic;

    class DynamicsWorld : public PhysicsWorld{
    public:
        DynamicsWorld();
        ~DynamicsWorld();

        typedef std::shared_ptr<void> RigidBodyPtr;

        void setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic);
        void addBody(btRigidBody* body);

        RigidBodyPtr createRigidBody(const btRigidBody::btRigidBodyConstructionInfo& info);

        void update();

        static void _destroyBody(void* body);

    private:
        std::mutex dynWorldMutex;

        static DynamicsWorld* _dynWorld;

        ScriptDataPacker<btRigidBody*> mBodyData;

        DynamicsWorldThreadLogic* mDynLogic;
    };
}
