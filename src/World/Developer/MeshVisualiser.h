#ifdef DEBUGGING_TOOLS
#pragma once

#include "World/Physics/PhysicsTypes.h"
#include "System/EnginePrerequisites.h"

#include <map>

namespace Ogre{
    class SceneManager;
    class SceneNode;
    class HlmsUnlitDatablock;
    class Vector3;
}

class dtNavMesh;

namespace AV{
    class Event;
    class NavMeshDebugDraw;

    /**
    A developer class which helps draw meshes, such as physics bodies, in the scene.

    It allows control over how these aspects are drawn.
    */
    class MeshVisualiser{
    public:
        MeshVisualiser();
        ~MeshVisualiser();

        void initialise(Ogre::SceneManager* sceneManager);

        void insertCollisionObjectChunk(const PhysicsTypes::CollisionChunkEntry& chunk);

        void insertPhysicsChunk(const PhysicsTypes::PhysicsChunkEntry& chunk);
        void destroyPhysicsChunk(const PhysicsTypes::PhysicsChunkEntry& chunk);

        void insertCollisionObject(uint8 collisionWorldId, const btCollisionObject* obj);
        void removeCollisionObject(uint8 collisionWorldId, const btCollisionObject* obj);

        void insertNavMesh(dtNavMesh* mesh);
        void removeNavMesh(dtNavMesh* mesh);

        void setCollisionObjectPosition(const Ogre::Vector3& pos, const btCollisionObject* obj);

        enum MeshGroupType : uint32_t{
            PhysicsChunk = 1u << 0u
            //physicsChunk = 1u << 1u
        };

        void setMeshGroupVisible(MeshGroupType type, bool visible);
        void setOverrideVisible(bool visible);

    private:
        Ogre::SceneManager* mSceneManager;
        Ogre::SceneNode* mParentNode;
        Ogre::SceneNode* mPhysicsChunkNode;
        Ogre::SceneNode* mCollisionObjectsChunkNode;
        Ogre::SceneNode* mNavMeshObjectNode;
        Ogre::SceneNode* mCollisionWorldObjectNodes[MAX_COLLISION_WORLDS];

        typedef std::pair<std::vector<PhysicsTypes::ShapePtr>*, std::vector<btRigidBody*>*> PhysicsChunkContainer;

        std::map<PhysicsChunkContainer, Ogre::SceneNode*> mAttachedPhysicsChunks;
        std::map<PhysicsTypes::CollisionChunkEntry, Ogre::SceneNode*> mAttachedCollisionObjectChunks;
        //A map of attached collision objects. This includes all collision worlds at once.
        std::map<const btCollisionObject*, Ogre::SceneNode*> mAttachedCollisionObjects;
        std::map<dtNavMesh*, Ogre::SceneNode*> mAttachedNavMeshes;

        //The number of catagories of meshes which can exist at a time.
        //This might be something like dynamic bodies, static bodies, nav meshes, etc.
        static const int NUM_CATEGORIES = 5;

        static const char* mDatablockNames[NUM_CATEGORIES];
        Ogre::HlmsUnlitDatablock* mCategoryDatablocks[NUM_CATEGORIES];
        Ogre::HlmsUnlitDatablock* mNavMeshDatablock;

        Ogre::SceneNode* _createSceneNode(Ogre::SceneNode* parent, const btCollisionObject* obj, uint8 datablockId);

        void _repositionMeshesOriginShift(const Ogre::Vector3& offset);
        bool worldEventReceiver(const Event &e);

        bool mVisibleOverride;

        bool debuggerToolsReceiver(const Event &e);

        std::shared_ptr<NavMeshDebugDraw> mNavMeshDebugDraw;
    };
}

#endif
