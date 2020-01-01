//#ifdef fsdkjfklsj

#include "Rect2dManager.h"

#include "OgreSceneManager.h"
#include "OgreSceneNode.h"

#include "Gui/Rect2d/Rect2d.h"
#include "Gui/Rect2d/Rect2dMovable.h"
#include "Gui/Texture2d/MovableTexture.h"
#include "Gui/Texture2d/MovableTextureManager.h"
#include "System/BaseSingleton.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/SystemEvent.h"

#include "System/SystemSetup/SystemSettings.h"

#include "OgreHlmsUnlit.h"
#include "OgreHlmsUnlitDatablock.h"
#include "OgreHlmsManager.h"

namespace AV{
    Rect2dManager::Rect2dManager(){
        EventDispatcher::subscribe(EventType::System, AV_BIND(Rect2dManager::systemEventReceiver));

        MovableTexture::_updateScreenSize(SystemSettings::getDefaultWindowWidth(), SystemSettings::getDefaultWindowHeight());

    }

    Rect2dManager::~Rect2dManager(){

    }

    void Rect2dManager::initialise(Ogre::SceneManager* sceneManager){
        mSceneManager = sceneManager;

        mParentNode = mSceneManager->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_DYNAMIC); //TODO destroy this on shutdown.
        mParentNode->setPosition(Ogre::Vector3::ZERO);

        Rect2d::_cacheDefaultDatablock();
    }

    Rect2dPtr Rect2dManager::createRect2d(LayerId layer){
        Rect2dMovable* rectMov = static_cast<Rect2dMovable*>(
            mSceneManager->createMovableObject("Rect2dMovable", &mSceneManager->_getEntityMemoryManager(Ogre::SCENE_DYNAMIC))
        );

        Ogre::SceneNode* node = mParentNode->createChildSceneNode(Ogre::SCENE_DYNAMIC);
        node->attachObject(rectMov);

        Rect2d* rec2d = new Rect2d(node, rectMov);

        Rect2dPtr sharedPtr(rec2d, _destroyRect2d);

        mCurrentRects[layer].insert(rec2d);
        rec2d->mLayer = layer;

        return sharedPtr;
    }

    bool Rect2dManager::isRectInLayer(Rect2dPtr rec, LayerId layer){
        return mCurrentRects[layer].find(rec.get()) != mCurrentRects[layer].end();
    }

    void Rect2dManager::setRectLayer(Rect2dPtr rec, LayerId layer){
        LayerId prev = rec->mLayer;
        Rect2d* recPtr = rec.get();
        mCurrentRects[prev].erase(recPtr);

        mCurrentRects[layer].insert(recPtr);

        recPtr->mLayer = layer;

    }

    void Rect2dManager::_destroyRect2d(Rect2d* rect){

        auto man = BaseSingleton::getRect2dManager();

        rect->destroy(man->mSceneManager);

        man->mCurrentRects[rect->mLayer].erase(rect);

        delete rect;
    }

    bool Rect2dManager::systemEventReceiver(const Event& e){
        const SystemEvent& event = (SystemEvent&)e;
        if(event.eventCategory() == SystemEventCategory::WindowResize){
            const SystemEventWindowResize& rEvent = (SystemEventWindowResize&)e;

            Rect2d::_updateScreenSize(rEvent.width, rEvent.height);

            for(const auto& s : mCurrentRects){
                for(Rect2d* t : s.second){
                    t->_notifyResize();
                }
            }

        }
        return true;
    }

    int Rect2dManager::getNumRects() const {
        int count = 0;
        for(const auto& i : mCurrentRects){
            count += i.second.size();
        }

        return count;
    }

    int Rect2dManager::getNumRectsForLayer(LayerId layer) const {
        if(mCurrentRects.find(layer) == mCurrentRects.end()) return 0;

        return mCurrentRects.at(layer).size();
    }
}

//#endif
