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

        @returns
        A boolean representing whether this piece of terrain was setup correctly.
        For instance, if this chunk coordinate doesn't contain any terrain data it will return false.
        */
        bool setup(const ChunkCoordinate& coord);

        //TODO Temporary.
        void provideSceneNode(Ogre::SceneNode* node);

    private:
        Ogre::Terra* mTerra;
        Ogre::SceneNode* mNode = 0;

        bool mSetupComplete = false;
        Ogre::String mTerrainGroupName;

        void _createTerrainResourceGroup(const Ogre::String& dirPath, const Ogre::String& groupName);

        Ogre::HlmsDatablock* _getTerrainDatablock(const ChunkCoordinate& coord);
        void _resetVals();
    };
}
