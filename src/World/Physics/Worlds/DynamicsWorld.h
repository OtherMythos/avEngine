#pragma once

#include "PhysicsWorld.h"
#include "OgreVector3.h"

#include "btBulletDynamicsCommon.h"
#include "World/Entity/eId.h"

#include "World/Physics/PhysicsTypes.h"

#include <mutex>
#include <memory>
#include <set>

#include "Scripting/ScriptDataPacker.h"

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

        void addBody(PhysicsTypes::RigidBodyPtr body);
        void removeBody(PhysicsTypes::RigidBodyPtr body);
        void setBodyPosition(PhysicsTypes::RigidBodyPtr body, btVector3 pos);
        uint32_t addPhysicsChunk(PhysicsTypes::PhysicsChunkEntry chunk);
        void removePhysicsChunk(uint32_t chunkId, bool requestWorldRemoval = true);

        /**
        Add a terrain body into the world.
        For now it takes just a regular rigid body pointer.
        */
        void addTerrainBody(btRigidBody* terrain);

        bool bodyInWorld(PhysicsTypes::RigidBodyPtr body) const;
        bool attachEntityToBody(PhysicsTypes::RigidBodyPtr body, eId e);
        void detatchEntityFromBody(PhysicsTypes::RigidBodyPtr body);
        bool attachMeshToBody(PhysicsTypes::RigidBodyPtr body, Ogre::SceneNode* meshNode);
        void detachMeshFromBody(PhysicsTypes::RigidBodyPtr body);
        BodyAttachObjectType getBodyBindType(PhysicsTypes::RigidBodyPtr body);

        void setBodyLinearFactor(PhysicsTypes::RigidBodyPtr body, btVector3 factor);
        void setBodyLinearVelocity(PhysicsTypes::RigidBodyPtr body, btVector3 velocity);

        void notifyOriginShift(Ogre::Vector3 offset);

        const std::vector<EntityTransformData>& getEntityTransformData() { return mEntityTransformData; }

        void update();

    private:
        std::set<btRigidBody*> mBodiesInWorld;
        std::map<btRigidBody*, eId> mEntitiesInWorld;
        std::map<btRigidBody*, Ogre::SceneNode*> mMeshesInWorld;
        std::vector<EntityTransformData> mEntityTransformData;
        std::vector<MeshTransformData> mMeshTransformData;
        std::set<btRigidBody*> mIgnoredBodies;
        std::vector<PhysicsTypes::PhysicsChunkEntry> mPhysicsChunksInWorld;

        void _resetBufferEntries(btRigidBody* b);
        bool _attachToBody(btRigidBody* body, DynamicsWorld::BodyAttachObjectType type);
        void _detatchFromBody(btRigidBody* body);
        void _destroyBodyInternal(btRigidBody* bdy);

        uint32_t _findPhysicsChunksHole();

        bool mShiftPerformedLastFrame = false;

        /**
        Remove a body from the destruction world.
        This should only be called by shared pointers during their destruction.
        */
        static void _removeBody(btRigidBody* bdy);
        static ScriptDataPacker<PhysicsTypes::RigidBodyEntry>* mBodyData;
        static DynamicsWorld* dynWorld;

        inline bool _shouldIgnoreBody(btRigidBody* bdy);
        inline bool _bodyInWorld(btRigidBody* bdy) const;
        inline void _deleteBodyPtr(btRigidBody* bdy);

        DynamicsWorldThreadLogic* mDynLogic;
    };
}
