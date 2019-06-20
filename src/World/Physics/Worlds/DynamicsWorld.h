#pragma once

#include "PhysicsWorld.h"

#include "btBulletDynamicsCommon.h"
#include "World/Entity/eId.h"

#include "World/Physics/PhysicsBodyConstructor.h"

#include <mutex>
#include <memory>
#include <set>

namespace AV{
    class DynamicsWorldThreadLogic;
    class PhysicsBodyConstructor;

    class DynamicsWorld : public PhysicsWorld{
        friend PhysicsBodyConstructor;
    public:
        DynamicsWorld();
        ~DynamicsWorld();

        enum class BodyAttachObjectType{
            OBJECT_TYPE_NONE,
            OBJECT_TYPE_ENTITY,
            OBJECT_TYPE_MESH
        };

        struct EntityTransformData{
            eId entity;
            btVector3 pos;
        };

        void setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic);

        void addBody(PhysicsBodyConstructor::RigidBodyPtr body);
        void removeBody(PhysicsBodyConstructor::RigidBodyPtr body);

        bool bodyInWorld(PhysicsBodyConstructor::RigidBodyPtr body);
        bool attachEntityToBody(PhysicsBodyConstructor::RigidBodyPtr body, eId e);
        void detatchEntityFromBody(PhysicsBodyConstructor::RigidBodyPtr body);
        BodyAttachObjectType getBodyBindType(PhysicsBodyConstructor::RigidBodyPtr body);

        const std::vector<EntityTransformData>& getEntityTransformData() { return mEntityTransformData; }

        void update();

    private:
        std::mutex dynWorldMutex;

        std::set<btRigidBody*> mBodiesInWorld;
        std::map<btRigidBody*, eId> mEntitiesInWorld;
        std::vector<EntityTransformData> mEntityTransformData;

        void _resetBufferEntries(btRigidBody* b);
        bool _attachToBody(btRigidBody* body, DynamicsWorld::BodyAttachObjectType type);
        void _detatchFromBody(btRigidBody* body);

        static ScriptDataPacker<PhysicsBodyConstructor::RigidBodyEntry>* mBodyData;

        DynamicsWorldThreadLogic* mDynLogic;
    };
}
