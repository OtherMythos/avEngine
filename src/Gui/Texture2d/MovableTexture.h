#pragma once

namespace Ogre{
    class HlmsUnlit;
    class HlmsUnlitDatablock;
    class HlmsSamplerblock;
}

#include "OgrePrerequisites.h"
#include "Gui/Rect2d/Rect2d.h"

namespace AV{
    class Rect2dMovable;
    class MovableTextureManager;
    class CompositorPassRect2d;
    class Rect2dManager;

    class MovableTexture : public Rect2d{
        friend MovableTextureManager;
        friend Rect2dManager;
        friend CompositorPassRect2d;

    private:
        //Not to be called directly. Instead call MovableTextureManager::createTexture.
        MovableTexture(const Ogre::String& textureName, const Ogre::String& textureGroup,
            Ogre::SceneNode* sceneNode, Rect2dMovable* movable);
        ~MovableTexture();
    public:

        void destroy(Ogre::SceneManager* sceneManager);

        //Only intended to be called by the MovableTextureManager after a screen size update.
        //void _notifyResize();
        //static void _updateScreenSize(int width, int height);
        //A function to cache the sampler block used for pixel perfect scaling.
        //This is done so as not to keep requesting it each time a texture decides it needs it.
        static void _cacheSamplerblock(const Ogre::HlmsSamplerblock* sampler);

        //void setPosition(float x, float y);

        //void setSize(float w, float h);
        //void setWidth(float w);
        //void setHeight(float h);

        void setTexture(const Ogre::String& textureName, const Ogre::String& textureGroup = "General");
        void setTexture(Ogre::TexturePtr tex);

        void setSectionScale(float scaleX, float scaleY, float posX, float posY);

        //Ogre::uint8 getLayer() const { return mLayer; }


    private:
        //Ogre::SceneNode* mSceneNode;

        //static int screenWidth, screenHeight;

        //Rect2dMovable* mMovable;

        //Rect2dMovable* getMovable() const { return mMovable; }

        //Ogre::uint8 mLayer;

        void _updateDatablock(Ogre::TexturePtr tex);
        void _createDatablock(Ogre::HlmsUnlit* unlit, Ogre::TexturePtr tex);

        // float posX, posY;
        // float width, height;

        bool mAnimationEnabled = false;

        // void _recalculatePosition();
        // void _recalculateSize();

        Ogre::HlmsUnlitDatablock* mTextureDatablock;
        static const Ogre::HlmsSamplerblock* mSampler;
    };
}
