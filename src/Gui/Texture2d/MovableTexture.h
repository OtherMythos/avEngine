#pragma once

namespace Ogre{
    class HlmsUnlit;
    class HlmsUnlitDatablock;
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

        void setPosition(float x, float y);

        void setSize(float w, float h);
        void setWidth(float w);
        void setHeight(float h);

        void setTexture(const Ogre::String& textureName, const Ogre::String& textureGroup = "General");
        void setTexture(Ogre::TexturePtr tex);

    private:
        Ogre::SceneNode* mSceneNode;

        Rect2dMovable* mMovable;

        void _updateDatablock(Ogre::TexturePtr tex);
        void _createDatablock(Ogre::HlmsUnlit* unlit, Ogre::TexturePtr tex);

        float posX, posY;
        float width, height;

        void _recalculatePosition();
        void _recalculateSize();

        Ogre::HlmsUnlitDatablock* mTextureDatablock;
    };
}
