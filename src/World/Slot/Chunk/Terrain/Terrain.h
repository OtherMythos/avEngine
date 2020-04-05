#pragma once

namespace Ogre{
    class Terra;
    class SceneNode;
    class HlmsTerraDatablock;
}

#include "World/Slot/ChunkCoordinate.h"
#include "OgreString.h"

class btHeightfieldTerrainShape;
class btRigidBody;

namespace AV{
    class TerrainManager;

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
        bool setup(const ChunkCoordinate& coord, TerrainManager& terrainManager);

        //Teardown the terrain. Setup should have been called before calling this.
        void teardown();

        //TODO Temporary.
        void provideSceneNode(Ogre::SceneNode* node);

        static void clearShadowTexture();

        btRigidBody* getTerrainBody() const { return mTerrainBody; }

    private:
        Ogre::Terra* mTerra = 0;
        Ogre::SceneNode* mNode = 0;
        Ogre::HlmsDatablock* mCurrentSetDatablock = 0;

        bool mSetupComplete = false;
        Ogre::String mTerrainGroupName;
        Ogre::String mGroupPath;

        btHeightfieldTerrainShape* mTerrainShape = 0;
        btRigidBody* mTerrainBody = 0;

        void _createTerrainResourceGroup(const Ogre::String& dirPath, const Ogre::String& groupName);

        //Determine if a datablock requires a blend texture, and if it does applies it.
        void _applyBlendMapToDatablock(Ogre::HlmsTerraDatablock* db);

        Ogre::HlmsDatablock* _getTerrainDatablock(const ChunkCoordinate& coord);
        void _resetVals();

        //Get a pointer to the blank shadow map.
        //The first time this is called it will be created.
        Ogre::TextureGpu* _getBlankShadowMap();

        /**
        Clear the static shadow texture, to make sure it's deleted before shutdown.
        I don't do any reference counting or anything like that because I don't want to risk deleting and re-creating the texture in the same world.
        If the texture is requested it will live until the world is destroyed, when this is called.
        */
        static Ogre::TextureGpu* mShadowMap;
    };
}
