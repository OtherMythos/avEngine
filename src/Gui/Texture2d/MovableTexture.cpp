#include "MovableTexture.h"

#include "Gui/Rect2d/Rect2dMovable.h"

#include "OgreHlmsUnlit.h"
#include "OgreHlmsUnlitDatablock.h"
#include "OgreHlmsManager.h"
#include "OgreStringConverter.h"

//#include "OgreTextureManager.h"
#include "OgreTextureGpuManager.h"

#include "Logger/Log.h"

namespace AV{

    //int MovableTexture::screenWidth = 1600;
    //int MovableTexture::screenHeight = 1200;
    const Ogre::HlmsSamplerblock* MovableTexture::mSampler = 0;

    MovableTexture::MovableTexture(const Ogre::String& textureName, const Ogre::String& textureGroup
        , Ogre::SceneNode* sceneNode, Rect2dMovable* movable)
        : //mSceneNode(sceneNode),
          //mMovable(movable),

          /*posX(0.0f),
          posY(0.0f),
          width(100.0f),
          height(100.0f),*/
          mTextureDatablock(0),
          Rect2d(sceneNode, movable){

        setTexture(textureName, textureGroup);

        //_recalculatePosition();
        //_recalculateSize();

    }

    MovableTexture::~MovableTexture(){

    }

    void MovableTexture::_cacheSamplerblock(const Ogre::HlmsSamplerblock* sampler){
        mSampler = sampler;
    }

    void MovableTexture::destroy(Ogre::SceneManager* sceneManager){
        /*mSceneNode->detachAllObjects();

        sceneManager->destroyMovableObject(mMovable);
        sceneManager->destroySceneNode(mSceneNode);
        mMovable = 0;
        mSceneNode = 0;*/
        Rect2d::destroy(sceneManager);

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

    void MovableTexture::_createDatablock(Ogre::HlmsUnlit* unlit, Ogre::TextureGpu* tex){
        assert(!mTextureDatablock && "Cannot create a new datablock while the old one still exists.");
        assert(tex);

        const Ogre::String dbName("/movTex/db" + Ogre::StringConverter::toString(mMovable->getId()));

        //Enable alpha blending.
        Ogre::HlmsBlendblock bb;
        bb.mSeparateBlend = true;
        bb.mSourceBlendFactor = Ogre::SBF_SOURCE_ALPHA;
        bb.mDestBlendFactor = Ogre::SBF_ONE_MINUS_SOURCE_ALPHA;
        bb.mSourceBlendFactorAlpha = Ogre::SBF_ONE_MINUS_DEST_ALPHA;
        bb.mDestBlendFactorAlpha = Ogre::SBF_ONE;

        Ogre::HlmsDatablock* block =
        unlit->createDatablock(Ogre::IdString(dbName), dbName, Ogre::HlmsMacroblock(), bb, Ogre::HlmsParamVec());

        mTextureDatablock = dynamic_cast<Ogre::HlmsUnlitDatablock*>(block);

        //TODO only pass in the sampler block if pixel perfect is requested.
        //mTextureDatablock->setTexture(0, 0, tex, mSampler);
        mTextureDatablock->setTexture(0, "cat.jpg", mSampler); //Temporary name
    }

    void MovableTexture::_updateDatablock(Ogre::TextureGpu* tex){

        Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT);
        Ogre::HlmsUnlit* unlit = dynamic_cast<Ogre::HlmsUnlit*>(hlms);
        assert(unlit);

        if(tex){

            if(!mTextureDatablock){
                _createDatablock(unlit, tex);
            }else{
                //mTextureDatablock->setTexture(0, 0, tex);
                mTextureDatablock->setTexture(0, "cat.jpg"); //TODO it seems hlmsdatablocks don't allow direct pointers anymore. Lots of this will have to change in that case.
            }

            mMovable->setDatablock(mTextureDatablock);

        }else{
            //A bad pointer was passed in, so use the default datablock instead.
            mMovable->setDatablock(unlit->getDefaultDatablock());
        }


    }

    void MovableTexture::setTexture(const Ogre::String& textureName, const Ogre::String& textureGroup){
        Ogre::TextureGpu* tex;
        //TODO this should be a try catch.
        if(Ogre::ResourceGroupManager::getSingleton().resourceExists(textureGroup, textureName)){
            //tex = Ogre::TextureManager::getSingleton().load(textureName, textureGroup);
            tex = Ogre::Root::getSingleton().getRenderSystem()->getTextureGpuManager()->createOrRetrieveTexture(textureName, Ogre::GpuPageOutStrategy::Discard, 0, Ogre::TextureTypes::Type2D, textureGroup);
        }else{
            AV_WARN("No texture named {} in resource group {} was found.", textureName, textureGroup);
        }

        setTexture(tex);
    }

    void MovableTexture::setTexture(Ogre::TextureGpu* tex){

        _updateDatablock(tex);

    }

    void MovableTexture::setColour(Ogre::ColourValue col){
        if(!mTextureDatablock) return;

        mTextureDatablock->setUseColour(true);
        mTextureDatablock->setColour(col);
    }

    /*void MovableTexture::setPosition(float x, float y){
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
    */
    void MovableTexture::setSectionScale(float scaleX, float scaleY, float posX, float posY){
        if(mTextureDatablock){
            if(!mAnimationEnabled){
                //Don't do this each time as it will trigger a flush.
                mTextureDatablock->setEnableAnimationMatrix(0, true);
                mAnimationEnabled = true;
            }

            mTextureDatablock->setAnimationMatrix(0,
                Ogre::Matrix4(
                    scaleX, 0, 0, posX,
                    0, scaleY, 0, posY,
                    0, 0, 1, 0,
                    0, 0, 0, 1
                )
            );
        }
    }
/*
    void MovableTexture::_recalculateSize(){
        float newWidth = (width / screenWidth);
        float newHeight = (height / screenHeight);

        mSceneNode->setScale(newWidth * 2, newHeight * 2, 1);
    }

    void MovableTexture::_recalculatePosition(){
        float newX = ((posX / screenWidth)*2) - 1;
        float newY = -((posY / screenHeight*2) - 1); //Flipped so positive y coordinates move from the top left corner.

        mSceneNode->setPosition(newX, newY, 0);
    }

    void MovableTexture::_notifyResize(){
        _recalculateSize();
        _recalculatePosition();
    }

    void MovableTexture::_updateScreenSize(int width, int height){
        MovableTexture::screenWidth = width;
        MovableTexture::screenHeight = height;
    }*/
}
