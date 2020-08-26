#pragma once

#include "PhysicsWorld.h"
#include "System/EnginePrerequisites.h"

#include "World/Entity/eId.h"

#include "System/Util/DataPacker.h"

#include <unordered_map>

namespace Ogre{
    class SceneNode;
};

namespace AV{
    class DynamicsWorldThreadLogic;
    class PhysicsBodyConstructor;
    class SlotPosition;

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

        //Defines a thread safe copy for some queryable data. This will be mirrored on the main thread for each object in the world.
        struct DynamicsObjectWorldData{
            btVector3 position;
            btQuaternion orientation;
            btVector3 linearVelocity;
        };

        void setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic);

        void addBody(PhysicsTypes::RigidBodyPtr body);
        void removeBody(PhysicsTypes::RigidBodyPtr body);
        void setBodyPosition(PhysicsTypes::RigidBodyPtr body, btVector3 pos);
        uint32 addPhysicsChunk(const PhysicsTypes::PhysicsChunkEntry& chunk);
        void removePhysicsChunk(uint32 chunkId, bool requestWorldRemoval = true);

        /**
        Add a terrain body into the world.

        @param terrain
        The terrain rigid body. This should be a terrain rigid body object.
        @param x
        The chunk x of the terrain.
        @param y
        The chunk y of the terrain.
        */
        void addTerrainBody(btRigidBody* terrain, int x, int y);

        bool bodyInWorld(PhysicsTypes::RigidBodyPtr body) const;
        bool attachEntityToBody(PhysicsTypes::RigidBodyPtr body, eId e);
        void detatchEntityFromBody(PhysicsTypes::RigidBodyPtr body);
        bool attachMeshToBody(PhysicsTypes::RigidBodyPtr body, Ogre::SceneNode* meshNode);
        void detachMeshFromBody(PhysicsTypes::RigidBodyPtr body);
        BodyAttachObjectType getBodyBindType(PhysicsTypes::RigidBodyPtr body);

        void setBodyLinearFactor(PhysicsTypes::RigidBodyPtr body, btVector3 factor);
        void setBodyLinearVelocity(PhysicsTypes::RigidBodyPtr body, btVector3 velocity);

        void notifyOriginShift(const Ogre::Vector3& offset, const SlotPosition& newPos);

        const std::vector<EntityTransformData>& getEntityTransformData() { return mEntityTransformData; }

        void update();

        btVector3 getBodyPosition(PhysicsTypes::RigidBodyPtr body);
        btVector3 getBodyLinearVelocity(PhysicsTypes::RigidBodyPtr body);

    private:
        std::set<btRigidBody*> mBodiesInWorld;
        std::map<btRigidBody*, eId> mEntitiesInWorld;
        std::map<btRigidBody*, Ogre::SceneNode*> mMeshesInWorld;
        std::vector<EntityTransformData> mEntityTransformData;
        std::vector<MeshTransformData> mMeshTransformData;
        std::set<btRigidBody*> mIgnoredBodies;
        std::vector<PhysicsTypes::PhysicsChunkEntry> mPhysicsChunksInWorld;
        //A thread safe copy of notable object properties.
        std::unordered_map<btRigidBody*, DynamicsObjectWorldData> mObjectWorldData;

        void _resetBufferEntries(btRigidBody* b);
        bool _attachToBody(btRigidBody* body, DynamicsWorld::BodyAttachObjectType type);
        void _detatchFromBody(btRigidBody* body);
        void _destroyBodyInternal(btRigidBody* bdy);

        uint32 _findPhysicsChunksHole();

        /**
        Remove a body from the destruction world.
        This should only be called by shared pointers during their destruction.
        */
        static void _removeBody(btRigidBody* bdy);
        static DataPacker<PhysicsTypes::RigidBodyEntry>* mBodyData;
        static DynamicsWorld* dynWorld;

        inline bool _shouldIgnoreBody(btRigidBody* bdy);
        inline bool _bodyInWorld(btRigidBody* bdy) const;
        inline void _deleteBodyPtr(btRigidBody* bdy);

        DynamicsWorldThreadLogic* mDynLogic;
    };
}
