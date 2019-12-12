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

    }

    void CompositorPassRect2d::execute(const Ogre::Camera *lodCamera){
        static bool first = false;
        if(!first){
            //Provides an early example of how this will work. Not planned to remain there.
            tex = BaseSingleton::getMovableTextureManager()->createTexture("cat1.jpg", "General");
            tex2 = BaseSingleton::getMovableTextureManager()->createTexture("cat2.jpg", "General");
            tex2->setPosition(50, 50);

            first = true;
        }

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

        { //Temorary and horrible code. Will be removed.
            static int count = 0;
            count++;
            static bool inverse = false;
            if(count % 10 == 0) inverse = !inverse;

            {auto rectMov = inverse ? tex->getMovable() : tex2->getMovable();
                Ogre::RenderableArray::const_iterator itRend = rectMov->mRenderables.begin();
                Ogre::RenderableArray::const_iterator enRend = rectMov->mRenderables.end();

                while( itRend != enRend ){
                    mRenderQueue->addRenderableV2(0, 80, false, *itRend, rectMov);
                    ++itRend;
                }}
            {auto rectMov = !inverse ? tex->getMovable() : tex2->getMovable();
            Ogre::RenderableArray::const_iterator itRend = rectMov->mRenderables.begin();
            Ogre::RenderableArray::const_iterator enRend = rectMov->mRenderables.end();

            while( itRend != enRend ){
                mRenderQueue->addRenderableV2(0, 80, false, *itRend, rectMov);
                ++itRend;
            }}
        }

        const Ogre::uint8 firstRq = 80;
        const Ogre::uint8 lastRq = 81; //To target 80 has to be one higher.
        mRenderQueue->render(mSceneManager->getDestinationRenderSystem(), firstRq, lastRq, false, false);


        if(listener)
            listener->passPosExecute(this);

        profilingEnd();
    }
}
