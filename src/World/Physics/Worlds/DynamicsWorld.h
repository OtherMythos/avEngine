#pragma once

#include "PhysicsWorld.h"

#include "btBulletDynamicsCommon.h"
#include "World/Entity/eId.h"

#include "World/Physics/PhysicsBodyConstructor.h"

#include <mutex>
#include <memory>
#include <set>

namespace Ogre{
    class SceneNode;
};

namespace AV{
    class DynamicsWorldThreadLogic;
    class PhysicsBodyConstructor;

    /**
    Main thread exposure logic for the dynamics world.
    */
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
            btQuaternion orientation;
        };

        struct MeshTransformData{
            Ogre::SceneNode* meshNode;
            btVector3 pos;
            btQuaternion orientation;
        };

        void setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic);

        void addBody(PhysicsBodyConstructor::RigidBodyPtr body);
        void removeBody(PhysicsBodyConstructor::RigidBodyPtr body);
        void setBodyPosition(PhysicsBodyConstructor::RigidBodyPtr body, btVector3 pos);

        bool bodyInWorld(PhysicsBodyConstructor::RigidBodyPtr body) const;
        bool attachEntityToBody(PhysicsBodyConstructor::RigidBodyPtr body, eId e);
        void detatchEntityFromBody(PhysicsBodyConstructor::RigidBodyPtr body);
        bool attachMeshToBody(PhysicsBodyConstructor::RigidBodyPtr body, Ogre::SceneNode* meshNode);
        void detachMeshFromBody(PhysicsBodyConstructor::RigidBodyPtr body);
        BodyAttachObjectType getBodyBindType(PhysicsBodyConstructor::RigidBodyPtr body);

        const std::vector<EntityTransformData>& getEntityTransformData() { return mEntityTransformData; }

        void update();

    private:
        std::set<btRigidBody*> mBodiesInWorld;
        std::map<btRigidBody*, eId> mEntitiesInWorld;
        std::map<btRigidBody*, Ogre::SceneNode*> mMeshesInWorld;
        std::vector<EntityTransformData> mEntityTransformData;
        std::vector<MeshTransformData> mMeshTransformData;
        std::set<btRigidBody*> mIgnoredBodies;

        void _resetBufferEntries(btRigidBody* b);
        bool _attachToBody(btRigidBody* body, DynamicsWorld::BodyAttachObjectType type);
        void _detatchFromBody(btRigidBody* body);
        void _destroyBodyInternal(btRigidBody* bdy);

        static void _destroyBody(btRigidBody* bdy);
        static ScriptDataPacker<PhysicsBodyConstructor::RigidBodyEntry>* mBodyData;
        static DynamicsWorld* dynWorld;

        inline bool _shouldIgnoreBody(btRigidBody* bdy);
        inline bool _bodyInWorld(btRigidBody* bdy) const;
        inline void _deleteBodyPtr(btRigidBody* bdy);

        DynamicsWorldThreadLogic* mDynLogic;
    };
}
