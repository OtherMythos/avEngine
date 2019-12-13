#include "CompositorPassRect2d.h"

#include "CompositorPassRect2dDef.h"
#include "Compositor/OgreCompositorWorkspaceListener.h"
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorWorkspace.h"

#include "OgreRoot.h"
#include "Gui/Rect2d/Rect2dMovable.h"

#include "System/BaseSingleton.h"
#include "Gui/Texture2d/MovableTexture.h"
#include "Gui/Texture2d/MovableTextureManager.h"

#include "System/BaseSingleton.h"

#include <iostream>

namespace AV{

    static MovableTexturePtr tex;
    static MovableTexturePtr tex2;

    CompositorPassRect2d::CompositorPassRect2d(const CompositorPassRect2dDef* definition, Ogre::SceneManager *sceneManager, const Ogre::CompositorChannel& target, Ogre::CompositorNode *parentNode)
        : Ogre::CompositorPass(definition, target, parentNode),
          mSceneManager(sceneManager){

        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        //TODO check if this needs to be deleted manually.
        mRenderQueue = OGRE_NEW Ogre::RenderQueue(root->getHlmsManager(), sceneManager, sceneManager->getDestinationRenderSystem()->getVaoManager());
        mRenderQueue->setRenderQueueMode(80, Ogre::RenderQueue::Modes::FAST);
        mRenderQueue->setSortRenderQueue(80, Ogre::RenderQueue::RqSortMode::DisableSort);

        mMovableTextureManager = BaseSingleton::getMovableTextureManager();

    }

    void CompositorPassRect2d::execute(const Ogre::Camera *lodCamera){
        mRenderQueue->clear();

        //Execute a limited number of times?
        if(mNumPassesLeft != std::numeric_limits<Ogre::uint32>::max())
        {
            if(!mNumPassesLeft) return;
            --mNumPassesLeft;
        }

        profilingBegin();

        Ogre::CompositorWorkspaceListener *listener = mParentNode->getWorkspace()->getListener();
        if(listener)
            listener->passEarlyPreExecute(this);

        executeResourceTransitions();

        if(listener)
            listener->passPreExecute(this);

        //At the moment just draw depending on the order of this list.
        for(const auto& m : mMovableTextureManager->mCurrentTextures){
            for(MovableTexture* tex : m.second){
                Rect2dMovable* t = tex->getMovable();
                mRenderQueue->addRenderableV2(0, 80, false, t->mRenderables[0], t);
            }
        }


        const Ogre::uint8 firstRq = 80;
        const Ogre::uint8 lastRq = 81; //To target 80 has to be one higher.
        mRenderQueue->render(mSceneManager->getDestinationRenderSystem(), firstRq, lastRq, false, false);


        if(listener)
            listener->passPosExecute(this);

        profilingEnd();
    }
}
