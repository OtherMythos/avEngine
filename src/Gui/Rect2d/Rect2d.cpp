#include "Rect2d.h"

#include "Gui/Rect2d/Rect2dMovable.h"
#include "OgreHlmsUnlit.h"
#include "OgreHlmsUnlitDatablock.h"
#include "OgreHlmsManager.h"

#include "Ogre.h"

namespace AV{

    int Rect2d::screenWidth = 1600;
    int Rect2d::screenHeight = 1200;

    Ogre::HlmsDatablock* Rect2d::mDefaultDatablock = 0;

    Rect2d::Rect2d(Ogre::SceneNode* sceneNode, Rect2dMovable* movable)
        : mSceneNode(sceneNode),
          mMovable(movable),

          posX(0.0f),
          posY(0.0f),
          width(100.0f),
          height(100.0f) {

        _recalculatePosition();
        _recalculateSize();

        mMovable->setDatablock(mDefaultDatablock);

    }

    Rect2d::~Rect2d(){

    }

    void Rect2d::destroy(Ogre::SceneManager* sceneManager){
        mSceneNode->detachAllObjects();

        sceneManager->destroyMovableObject(mMovable);
        sceneManager->destroySceneNode(mSceneNode);
        mMovable = 0;
        mSceneNode = 0;
    }

    void Rect2d::setPosition(float x, float y){
        posX = x;
        posY = y;

        _recalculatePosition();
    }

    void Rect2d::setSize(float w, float h){
        width = w;
        height = h;

        _recalculateSize();
    }

    void Rect2d::setWidth(float w){
        width = w;

        _recalculateSize();
    }

    void Rect2d::setHeight(float h){
        height = h;

        _recalculateSize();
    }

    void Rect2d::_recalculateSize(){
        float newWidth = (width / screenWidth);
        float newHeight = (height / screenHeight);

        mSceneNode->setScale(newWidth * 2, newHeight * 2, 1);
    }

    void Rect2d::_recalculatePosition(){
        float newX = ((posX / screenWidth)*2) - 1;
        float newY = -((posY / screenHeight*2) - 1); //Flipped so positive y coordinates move from the top left corner.

        mSceneNode->setPosition(newX, newY, 0);
    }

    void Rect2d::_notifyResize(){
        _recalculateSize();
        _recalculatePosition();
    }

    void Rect2d::_updateScreenSize(int width, int height){
        Rect2d::screenWidth = width;
        Rect2d::screenHeight = height;
    }

    void Rect2d::_cacheDefaultDatablock(){
        Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT);
        Ogre::HlmsUnlit* unlit = dynamic_cast<Ogre::HlmsUnlit*>(hlms);
        mDefaultDatablock = unlit->getDefaultDatablock();
    }
}
