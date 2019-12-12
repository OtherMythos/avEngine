#pragma once

namespace Ogre{
    class HlmsUnlit;
    class HlmsUnlitDatablock;
    class HlmsSamplerblock;
}

#include "OgrePrerequisites.h"

namespace AV{
    class Rect2dMovable;

    class MovableTexture{
    public:
        //Do not call this constructor directly. Instead call MovableTextureManager::createTexture.
        MovableTexture(const Ogre::String& textureName, const Ogre::String& textureGroup,
                    Ogre::SceneNode* sceneNode, Rect2dMovable* movable);
        ~MovableTexture();

        void destroy(Ogre::SceneManager* sceneManager);

        //Only intended to be called by the MovableTextureManager after a screen size update.
        void _notifyResize();
        static void _updateScreenSize(int width, int height);
        //A function to cache the sampler block used for pixel perfect scaling.
        //This is done so as not to keep requesting it each time a texture decides it needs it.
        static void _cacheSamplerblock(const Ogre::HlmsSamplerblock* sampler);

        void setPosition(float x, float y);

        void setSize(float w, float h);
        void setWidth(float w);
        void setHeight(float h);

        void setTexture(const Ogre::String& textureName, const Ogre::String& textureGroup = "General");
        void setTexture(Ogre::TexturePtr tex);

        void setSectionScale(float scaleX, float scaleY, float posX, float posY);

        //TODO remove
        Rect2dMovable* getMovable() const { return mMovable; }

    private:
        Ogre::SceneNode* mSceneNode;

        static int screenWidth, screenHeight;

        Rect2dMovable* mMovable;

        void _updateDatablock(Ogre::TexturePtr tex);
        void _createDatablock(Ogre::HlmsUnlit* unlit, Ogre::TexturePtr tex);

        float posX, posY;
        float width, height;

        bool mAnimationEnabled = false;

        void _recalculatePosition();
        void _recalculateSize();

        Ogre::HlmsUnlitDatablock* mTextureDatablock;
        static const Ogre::HlmsSamplerblock* mSampler;
    };
}
