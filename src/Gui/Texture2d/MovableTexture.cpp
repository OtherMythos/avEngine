#include "MovableTexture.h"

#include "Gui/Rect2d/Rect2dMovable.h"

namespace AV{

    //TODO temporary until I can get this populated with an event.
    #define WINDOW_WIDTH 1600.0f
    #define WINDOW_HEIGHT 1200.0f

    MovableTexture::MovableTexture(Ogre::SceneNode* sceneNode, Rect2dMovable* movable)
        : mSceneNode(sceneNode),
          mMovable(movable),

          posX(0.0f),
          posY(0.0f),
          width(100.0f),
          height(100.0f) {

        movable->setDatablock("HlmsUnlit1");

        _recalculatePosition();
        _recalculateSize();

    }

    MovableTexture::~MovableTexture(){

    }

    void MovableTexture::setPosition(float x, float y){
        posX = x;
        posY = y;

        _recalculatePosition();
    }

    void MovableTexture::setSize(float w, float h){
        width = w;
        height = h;

        _recalculateSize();
    }

    void MovableTexture::setWidth(float w){
        width = w;

        _recalculateSize();
    }

    void MovableTexture::setHeight(float h){
        height = h;

        _recalculateSize();
    }

    void MovableTexture::_recalculateSize(){
        float newWidth = (width / WINDOW_WIDTH);
        float newHeight = (height / WINDOW_HEIGHT);

        mSceneNode->setScale(newWidth, newHeight, 1);
    }

    void MovableTexture::_recalculatePosition(){
        float newX = (posX / WINDOW_WIDTH) - 1;
        float newY = (posY / WINDOW_HEIGHT) - 1;

        mSceneNode->setPosition(newX, 1, newY);
    }
}
