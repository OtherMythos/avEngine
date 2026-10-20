#ifdef DEBUGGING_TOOLS
#pragma once

#include "Physics/PhysicsTypes.h"
#include "System/EnginePrerequisites.h"

#include <map>
#include <cstdint>

namespace Ogre{
    class SceneManager;
    class SceneNode;
    class HlmsUnlitDatablock;
    class Vector3;
}

namespace AV{
    class Event;

    /**
    A developer class which helps draw meshes, such as physics bodies, in the scene.

    It allows control over how these aspects are drawn.
    */
    class MeshVisualiser{
    public:
        MeshVisualiser();
        ~MeshVisualiser();

        void initialise(Ogre::SceneManager* sceneManager);

        void insertCollisionObject(uint8 collisionWorldId, const btCollisionObject* obj);
        void removeCollisionObject(uint8 collisionWorldId, const btCollisionObject* obj);

        void setCollisionObjectPosition(const Ogre::Vector3& pos, const btCollisionObject* obj);

        void setOverrideVisible(bool visible);
        void setRenderQueueForMeshes(uint8 meshGroup);

    private:
        Ogre::SceneManager* mSceneManager;
        Ogre::SceneNode* mParentNode;
        Ogre::SceneNode* mCollisionWorldObjectNodes[MAX_COLLISION_WORLDS];
        //A map of attached collision objects. This includes all collision worlds at once.
        std::map<const btCollisionObject*, Ogre::SceneNode*> mAttachedCollisionObjects;

        //One category per collision world.
        static const int NUM_CATEGORIES = MAX_COLLISION_WORLDS;

        static const char* mDatablockNames[NUM_CATEGORIES];
        Ogre::HlmsUnlitDatablock* mCategoryDatablocks[NUM_CATEGORIES];

        Ogre::SceneNode* _createSceneNode(Ogre::SceneNode* parent, const btCollisionObject* obj, uint8 datablockId);

        bool mVisibleOverride;

        bool debuggerToolsReceiver(const Event &e);

        uint8 mRenderQueue;
    };
}

#endif
