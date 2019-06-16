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
#include <set>

namespace AV{
    class DynamicsWorldThreadLogic;

    class DynamicsWorld : public PhysicsWorld{
    public:
        DynamicsWorld();
        ~DynamicsWorld();

        enum class BodyAttachObjectType{
            OBJECT_TYPE_NONE,
            OBJECT_TYPE_ENTITY,
            OBJECT_TYPE_MESH
        };

        typedef std::shared_ptr<void> RigidBodyPtr;

        void setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic);

        void addBody(DynamicsWorld::RigidBodyPtr body);
        void removeBody(DynamicsWorld::RigidBodyPtr body);

        bool bodyInWorld(DynamicsWorld::RigidBodyPtr body);
        void attachObjectToBody(DynamicsWorld::RigidBodyPtr body, DynamicsWorld::BodyAttachObjectType type);

        RigidBodyPtr createRigidBody(btRigidBody::btRigidBodyConstructionInfo& info, PhysicsShapeManager::ShapePtr shape);

        void update();

        static void _destroyBody(void* body);

    private:
        std::mutex dynWorldMutex;

        typedef std::pair<btRigidBody*, PhysicsShapeManager::ShapePtr> rigidBodyEntry;

        std::set<btRigidBody*> mBodiesInWorld;

        void _resetBufferEntries(btRigidBody* b);

        static DynamicsWorld* _dynWorld;

        ScriptDataPacker<rigidBodyEntry> mBodyData;

        DynamicsWorldThreadLogic* mDynLogic;
    };
}
