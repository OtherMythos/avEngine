#pragma once

namespace Ogre{
    class Terra;
    class SceneNode;
}

#include "World/Slot/ChunkCoordinate.h"
#include "OgreString.h"

namespace AV{
    class Terrain{
    public:
        Terrain();
        ~Terrain();

        void update();
        /**
        Setup the terrain, letting it know which chunk it represents.
        */
        void setup(const ChunkCoordinate& coord);

        //TODO Temporary.
        void provideSceneNode(Ogre::SceneNode* node);

    private:
        Ogre::Terra* mTerra;
        Ogre::SceneNode* mNode = 0;

        bool mChunkHasTerrain = false;
        Ogre::String mTerrainGroupName;

        ChunkCoordinate mCurrentChunk;

        void _createTerrainResourceGroup(const Ogre::String& dirPath, const Ogre::String& groupName);
    };
}
