#include "MovableTexture.h"

#include "Gui/Rect2d/Rect2dMovable.h"

#include "OgreHlmsUnlit.h"
#include "OgreHlmsUnlitDatablock.h"
#include "OgreHlmsManager.h"
#include "OgreStringConverter.h"

#include "OgreTextureManager.h"

#include "Logger/Log.h"

namespace AV{

    //TODO temporary until I can get this populated with an event.
    #define WINDOW_WIDTH 1600.0f
    #define WINDOW_HEIGHT 1200.0f

    MovableTexture::MovableTexture(const Ogre::String& textureName, const Ogre::String& textureGroup
        , Ogre::SceneNode* sceneNode, Rect2dMovable* movable)
        : mSceneNode(sceneNode),
          mMovable(movable),

          posX(0.0f),
          posY(0.0f),
          width(100.0f),
          height(100.0f),
          mTextureDatablock(0) {

        setTexture(textureName, textureGroup);

        _recalculatePosition();
        _recalculateSize();

    }

    MovableTexture::~MovableTexture(){

    }

    void MovableTexture::destroy(Ogre::SceneManager* sceneManager){
        mSceneNode->detachAllObjects();

        sceneManager->destroyMovableObject(mMovable);
        sceneManager->destroySceneNode(mSceneNode);
        mMovable = 0;
        mSceneNode = 0;

        if(mTextureDatablock){
            Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT);
            Ogre::HlmsUnlit* unlit = dynamic_cast<Ogre::HlmsUnlit*>(hlms);

            const Ogre::String* s = mTextureDatablock->getNameStr();
            assert( !s->empty() );

            //For some reason this only accepts a name string and not a pointer.
            //By my understanding you can't just directly call delete on the datablock pointer because the manager needs to know about it.
            unlit->destroyDatablock(*s);
        }
    }

    void MovableTexture::_createDatablock(Ogre::HlmsUnlit* unlit, Ogre::TexturePtr tex){
        assert(!mTextureDatablock && "Cannot create a new datablock while the old one still exists. One cannot live while the other survives.");
        assert(tex);

        const Ogre::String dbName("/movTex/db" + Ogre::StringConverter::toString(mMovable->getId()));
        Ogre::HlmsDatablock* block =
        unlit->createDatablock(Ogre::IdString(dbName), dbName, Ogre::HlmsMacroblock(), Ogre::HlmsBlendblock(), Ogre::HlmsParamVec());

        mTextureDatablock = dynamic_cast<Ogre::HlmsUnlitDatablock*>(block);

        mTextureDatablock->setTexture(0, 0, tex);
    }

    void MovableTexture::_updateDatablock(Ogre::TexturePtr tex){

        Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT);
        Ogre::HlmsUnlit* unlit = dynamic_cast<Ogre::HlmsUnlit*>(hlms);
        assert(unlit);

        if(tex){

            if(!mTextureDatablock){
                _createDatablock(unlit, tex);
            }else{
                mTextureDatablock->setTexture(0, 0, tex);
            }

            mMovable->setDatablock(mTextureDatablock);

        }else{
            //A bad pointer was passed in, so use the default datablock instead.
            mMovable->setDatablock(unlit->getDefaultDatablock());
        }


    }

    void MovableTexture::setTexture(const Ogre::String& textureName, const Ogre::String& textureGroup){
        Ogre::TexturePtr tex;
        if(Ogre::ResourceGroupManager::getSingleton().resourceExists(textureGroup, textureName)){
            tex = Ogre::TextureManager::getSingleton().load(textureName, textureGroup);
        }else{
            AV_WARN("No texture named {} in resource group {} was found.", textureName, textureGroup);
        }

        setTexture(tex);
    }

    void MovableTexture::setTexture(Ogre::TexturePtr tex){

        _updateDatablock(tex);

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
        float newY = -(posY / WINDOW_HEIGHT - 1); //Flipped so positive y coordinates move from the top left corner.

        mSceneNode->setPosition(newX, newY, 0);
    }
}
