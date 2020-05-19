#ifdef DEBUGGING_TOOLS
#pragma once

#include "World/Physics/PhysicsTypes.h"

#include <map>

namespace Ogre{
    class SceneManager;
    class SceneNode;
    class HlmsUnlitDatablock;
}

namespace AV{

    /**
    A developer class which helps draw meshes, such as physics bodies, in the scene.

    It allows control over how these aspects are drawn.
    */
    class MeshVisualiser{
    public:
        MeshVisualiser();
        ~MeshVisualiser();

        void initialise(Ogre::SceneManager* sceneManager);

        void insertPhysicsChunk(const PhysicsTypes::PhysicsChunkEntry& chunk);
        void destroyPhysicsChunk(const PhysicsTypes::PhysicsChunkEntry& chunk);

    private:
        Ogre::SceneManager* mSceneManager;
        Ogre::SceneNode* mParentNode;

        std::map<PhysicsTypes::PhysicsChunkEntry, Ogre::SceneNode*> mAttachedPhysicsChunks;

        //The number of catagories of meshes which can exist at a time.
        //This might be something like dynamic bodies, static bodies, nav meshes, etc.
        static const int NUM_CATEGORIES = 1;

        static const char* mDatablockNames[NUM_CATEGORIES];
        Ogre::HlmsUnlitDatablock* mCategoryDatablocks[NUM_CATEGORIES];

        void _destroyPhysicsChunk(Ogre::SceneNode* node);
        void _recursiveDestroyNode(Ogre::SceneNode* node);
        void _destroyMovableObject(Ogre::SceneNode* node);
    };
}

#endif
