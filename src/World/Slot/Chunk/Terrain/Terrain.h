#pragma once

namespace Ogre{
    class Terra;
    class SceneNode;
    class HlmsTerraDatablock;
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

        //Teardown the terrain. Setup should have been called before calling this.
        void teardown();

        //TODO Temporary.
        void provideSceneNode(Ogre::SceneNode* node);

    private:
        Ogre::Terra* mTerra = 0;
        Ogre::SceneNode* mNode = 0;
        Ogre::HlmsDatablock* mCurrentSetDatablock = 0;

        bool mSetupComplete = false;
        Ogre::String mTerrainGroupName;
        Ogre::String mGroupPath;

        void _createTerrainResourceGroup(const Ogre::String& dirPath, const Ogre::String& groupName);

        //Determine if a datablock requires a blend texture, and if it does applies it.
        void _applyBlendMapToDatablock(Ogre::HlmsTerraDatablock* db);

        Ogre::HlmsDatablock* _getTerrainDatablock(const ChunkCoordinate& coord);
        void _resetVals();
    };
}
