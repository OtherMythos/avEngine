#include "CompositorPassRect2d.h"

#include "CompositorPassRect2dDef.h"
#include "Compositor/OgreCompositorWorkspaceListener.h"
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorWorkspace.h"

#include "OgreRoot.h"
#include "Gui/Rect2d/Rect2dMovable.h"

#include "System/BaseSingleton.h"
#include "Gui/Texture2d/MovableTexture.h"
#include "Gui/Rect2d/Rect2dManager.h"

#include "System/BaseSingleton.h"

namespace AV{

    static MovableTexturePtr tex;
    static MovableTexturePtr tex2;

    CompositorPassRect2d::CompositorPassRect2d(const CompositorPassRect2dDef* definition, Ogre::SceneManager *sceneManager, const Ogre::RenderTargetViewDef* target, Ogre::CompositorNode *parentNode)
        : Ogre::CompositorPass(definition, parentNode),
          mSceneManager(sceneManager){

        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        //TODO check if this needs to be deleted manually.
        mRenderQueue = OGRE_NEW Ogre::RenderQueue(root->getHlmsManager(), sceneManager, sceneManager->getDestinationRenderSystem()->getVaoManager());
        mRenderQueue->setRenderQueueMode(240, Ogre::RenderQueue::Modes::FAST);
        mRenderQueue->setSortRenderQueue(240, Ogre::RenderQueue::RqSortMode::DisableSort);

        mRect2dManager = BaseSingleton::getRect2dManager();

    }

    CompositorPassRect2d::~CompositorPassRect2d(){
        OGRE_DELETE mRenderQueue;
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

        notifyPassEarlyPreExecuteListeners();

        executeResourceTransitions();

        notifyPassPreExecuteListeners();

        mRenderQueue->renderPassPrepare(false, false);

        //At the moment just draw depending on the order of this list.
        for(const auto& m : mRect2dManager->mCurrentRects){
            for(Rect2d* rect : m.second){
                Rect2dMovable* t = rect->getMovable();
                if(!t->isVisible()) continue;
                mRenderQueue->addRenderableV2(0, 240, false, t->mRenderables[0], t);
            }
        }


        const Ogre::uint8 firstRq = 240;
        const Ogre::uint8 lastRq = 241; //To target 240 has to be one higher.
        mRenderQueue->render(mSceneManager->getDestinationRenderSystem(), firstRq, lastRq, false, false);
        mRenderQueue->frameEnded();


        notifyPassPosExecuteListeners();

        profilingEnd();
    }
}
