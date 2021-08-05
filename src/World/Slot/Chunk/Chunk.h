#pragma once

#include "World/Slot/ChunkCoordinate.h"
#include "World/Physics/PhysicsTypes.h"
#include "System/EnginePrerequisites.h"

#include "World/Nav/NavTypes.h"

namespace Ogre{
    class SceneNode;
    class SceneManager;
}
class dtNavMesh;

namespace AV{
    class TestModeSlotManagerNamespace;
    class PhysicsManager;
    class Terrain;
    class NavMeshManager;

    /**
    A constructed chunk in the world, derived from the construction instructions from a recipe.
    Chunks contain things such as static meshes, physics shapes, colliders and so on.
    Chunks provide an interface to activate and deactivate their contents.

    @remarks
    Chunks are constructed using the ChunkFactory, being given pointers to their constructed data to manage.
    */
    class Chunk{
        friend TestModeSlotManagerNamespace;
    public:
        struct NavMeshTileData{
            NavTilePtr tileId;
        };

        Chunk(const ChunkCoordinate &coord, std::shared_ptr<PhysicsManager> physicsManager, std::shared_ptr<NavMeshManager> navMeshManager, Ogre::SceneManager *sceneManager, Ogre::SceneNode *staticMeshes, PhysicsTypes::PhysicsChunkEntry physicsChunk, const PhysicsTypes::CollisionChunkEntry& collisionChunk, Terrain* terrain, std::vector<NavMeshTileData>& navTileData);
        virtual ~Chunk();

        /**
        Sets the chunk to be active.
        This will set meshes to visible and enable physics shapes.
        */
        virtual void activate();

        /**
        Deactivate the chunk.
        This will set the meshes to be invisible.
        */
        virtual void deActivate();

        /**
        Reposition this chunk.

        @remarks
        This position will be determined based off of the current origin.
        This function should not be called unless this chunk is within the safe bounds of the world origin, otherwise floating point precision issues might occur.
        */
        void reposition();

        void update();

        Ogre::SceneNode* getStaticMeshNode() { return mStaticMeshes; };
        Terrain* getTerrain() { return mTerrain; };

    private:
        ChunkCoordinate mChunkCoordinate;
        Terrain* mTerrain = 0;
        std::vector<NavMeshTileData> mNavMeshTiles;

        Ogre::SceneManager *mSceneManager;
        Ogre::SceneNode *mStaticMeshes;

        std::shared_ptr<PhysicsManager> mPhysicsManager;
        std::shared_ptr<NavMeshManager> mNavMeshManager;
        PhysicsTypes::PhysicsChunkEntry mPhysicsChunk;
        PhysicsTypes::CollisionChunkEntry mCollisionChunk;

        uint32_t currentPhysicsChunk = 0;
        uint32 currentCollisionObjectChunk = 0;

        bool mActive = false;
    };
}
