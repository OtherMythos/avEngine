#pragma once

#include "OgrePrerequisites.h"

namespace AV{
    class Rect2dMovable;
    class Rect2dManager;
    class CompositorPassRect2d;
    class Rect2dManager;

    class Rect2d{
        friend Rect2dManager;
        friend CompositorPassRect2d;

    protected:
        //Not to be called directly. Instead call Rect2dManager::createRect2d.
        Rect2d(Ogre::SceneNode* sceneNode, Rect2dMovable* movable);
        ~Rect2d();

        static void _cacheDefaultDatablock();
        Rect2dMovable* mMovable;
    public:

        void destroy(Ogre::SceneManager* sceneManager);

        //Only intended to be called by the Rect2dManager after a screen size update.
        void _notifyResize();
        static void _updateScreenSize(int width, int height);

        void setPosition(float x, float y);

        void setSize(float w, float h);
        void setWidth(float w);
        void setHeight(float h);
        void setVisible(bool visible);

        float getX() const { return posX; }
        float getY() const { return posY; }
        float getWidth() const { return width; }
        float getHeight() const { return height; }

        void setDatablock(Ogre::HlmsDatablock* datablock);

        Ogre::uint8 getLayer() const { return mLayer; }

        Ogre::uint8 mLayer;

    private:
        Ogre::SceneNode* mSceneNode;
        static Ogre::HlmsDatablock* mDefaultDatablock;

        static int screenWidth, screenHeight;

        Rect2dMovable* getMovable() const { return mMovable; }



        float posX, posY;
        float width, height;

        void _recalculatePosition();
        void _recalculateSize();
    };
}
