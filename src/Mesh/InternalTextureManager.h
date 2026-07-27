#pragma once

namespace Ogre{
    class TextureGpu;
}

namespace AV{

    /**
    A class to construct textures for internal use in the engine.
    */
    class InternalTextureManager{
    public:
        static void createTextures();
        static void shutdown();

        static Ogre::TextureGpu* getTerraShadowMap();

    private:
        static Ogre::TextureGpu* mShadowMapTexture;

    };

};