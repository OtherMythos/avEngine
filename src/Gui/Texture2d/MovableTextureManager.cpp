#include "MovableTextureManager.h"

#include "OgreSceneManager.h"
#include "OgreSceneNode.h"

#include "Gui/Rect2d/Rect2dMovable.h"
#include "MovableTexture.h"
#include "System/BaseSingleton.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/SystemEvent.h"

#include "System/SystemSetup/SystemSettings.h"

#include "OgreHlmsUnlit.h"
#include "OgreHlmsUnlitDatablock.h"
#include "OgreHlmsManager.h"

namespace AV{
    MovableTextureManager::MovableTextureManager(){
        EventDispatcher::subscribe(EventType::System, AV_BIND(MovableTextureManager::systemEventReceiver));

        MovableTexture::_updateScreenSize(SystemSettings::getDefaultWindowWidth(), SystemSettings::getDefaultWindowHeight());

    }

    MovableTextureManager::~MovableTextureManager(){

    }

    void MovableTextureManager::initialise(Ogre::SceneManager* sceneManager){
        mSceneManager = sceneManager;

        mParentNode = mSceneManager->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_DYNAMIC); //TODO destroy this on shutdown.
        mParentNode->setPosition(Ogre::Vector3::ZERO);

        Ogre::HlmsSamplerblock s;
        s.mMagFilter = Ogre::FO_POINT;
        const Ogre::HlmsSamplerblock* sP = Ogre::Root::getSingletonPtr()->getHlmsManager()->getSamplerblock(s);
        MovableTexture::_cacheSamplerblock(sP);
    }

    MovableTexturePtr MovableTextureManager::createTexture(const Ogre::String& resourceName, const Ogre::String& resourceGroup){
        Rect2dMovable* rectMov = static_cast<Rect2dMovable*>(
            mSceneManager->createMovableObject("Rect2dMovable", &mSceneManager->_getEntityMemoryManager(Ogre::SCENE_DYNAMIC))
        );

        Ogre::SceneNode* node = mParentNode->createChildSceneNode(Ogre::SCENE_DYNAMIC);
        node->attachObject(rectMov);


        //Soon these will be wrapped around a shared pointer to manage deletion.
        MovableTexture* movTex = new MovableTexture(resourceName, resourceGroup, node, rectMov);

        MovableTexturePtr sharedPtr(movTex, _destroyMovableTexture);

        mCurrentTextures.insert(movTex);

        return sharedPtr;
    }

    void MovableTextureManager::_destroyMovableTexture(MovableTexture* body){

        auto man = BaseSingleton::getMovableTextureManager();

        body->destroy(man->mSceneManager);

        man->mCurrentTextures.erase(body);

        delete body;
    }

    bool MovableTextureManager::systemEventReceiver(const Event& e){
        const SystemEvent& event = (SystemEvent&)e;
        if(event.eventCategory() == SystemEventCategory::WindowResize){
            const SystemEventWindowResize& rEvent = (SystemEventWindowResize&)e;

            MovableTexture::_updateScreenSize(rEvent.width, rEvent.height);

            for(MovableTexture* t : mCurrentTextures){
                t->_notifyResize();
            }

        }
        return true;
    }
}
