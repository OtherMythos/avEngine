#pragma once

namespace Ogre{
    class SceneNode;
}

namespace AV{
    class Rect2dMovable;

    class MovableTexture{
    public:
        //Do not call this constructor directly. Instead call MovableTextureManager::createTexture.
        MovableTexture(Ogre::SceneNode* sceneNode, Rect2dMovable* movable);
        ~MovableTexture();

        void setPosition(float x, float y);

        void setSize(float w, float h);
        void setWidth(float w);
        void setHeight(float h);

    private:
        Ogre::SceneNode* mSceneNode;

        Rect2dMovable* mMovable;

        float posX, posY;
        float width, height;

        void _recalculatePosition();
        void _recalculateSize();
    };
}
