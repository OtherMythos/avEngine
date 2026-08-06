#include "BaseSingleton.h"

namespace AV{
    std::shared_ptr<ScriptingStateManager> BaseSingleton::mScriptedStateManager;
    std::shared_ptr<OgreMeshManager> BaseSingleton::mOgreMeshManager;
    std::shared_ptr<Rect2dManager> BaseSingleton::mRect2dManager;
    std::shared_ptr<ValueRegistry> BaseSingleton::mGlobalRegistry;
    std::shared_ptr<EntityManager> BaseSingleton::mEntityManager;
    std::shared_ptr<PhysicsManager> BaseSingleton::mPhysicsManager;
    std::shared_ptr<InputManager> BaseSingleton::mInputManager;
    std::shared_ptr<GuiManager> BaseSingleton::mGuiManager;
    std::shared_ptr<GuiInputProcessor> BaseSingleton::mGuiInputProcessor;
    std::shared_ptr<ScriptManager> BaseSingleton::mScriptManager;
    std::shared_ptr<AnimationManager> BaseSingleton::mAnimationManager;
    std::shared_ptr<AudioManager> BaseSingleton::mAudioManager;

    Window* BaseSingleton::mWindow = 0;
    Base* BaseSingleton::mBase = 0;
    Ogre::SceneManager* BaseSingleton::mSceneManager = 0;

    PerformanceStats BaseSingleton::mPerformanceStats;

    #ifdef DEBUGGING_TOOLS
        DebugDrawer* BaseSingleton::mDebugDrawer = 0;
        std::shared_ptr<MeshVisualiser> BaseSingleton::mMeshVisualiser;

        void BaseSingleton::setupDebuggerTools(
            DebugDrawer* debugDrawer
        ){
            mDebugDrawer = debugDrawer;
        }

        DebugDrawer* BaseSingleton::getDebugDrawer(){
            return mDebugDrawer;
        }

        std::shared_ptr<MeshVisualiser> BaseSingleton::getMeshVisualiser(){
            return mMeshVisualiser;
        }
    #endif

    void BaseSingleton::initialise(
        Base* base,
        Window* window,
        std::shared_ptr<ScriptingStateManager> scriptedStateManager,
        std::shared_ptr<OgreMeshManager> ogreMeshManager,
        std::shared_ptr<Rect2dManager> rect2dManager,
        std::shared_ptr<ValueRegistry> valueRegistry,
        std::shared_ptr<InputManager> inputManager,
        std::shared_ptr<GuiManager> guiManager,
        std::shared_ptr<GuiInputProcessor> guiInputProcessor,
        std::shared_ptr<ScriptManager> scriptManager,
        std::shared_ptr<AnimationManager> animationManager,
        std::shared_ptr<AudioManager> audioManager
    ){

        mWindow = window;
        mBase = base;
        mScriptedStateManager = scriptedStateManager;
        mOgreMeshManager = ogreMeshManager;
        mRect2dManager = rect2dManager;
        mGlobalRegistry = valueRegistry;
        mInputManager = inputManager;
        mGuiManager = guiManager;
        mGuiInputProcessor = guiInputProcessor;
        mScriptManager = scriptManager;
        mAnimationManager = animationManager;
        mAudioManager = audioManager;
    }

    std::shared_ptr<ScriptingStateManager> BaseSingleton::getScriptedStateManager(){
        return mScriptedStateManager;
    }

    std::shared_ptr<OgreMeshManager> BaseSingleton::getOgreMeshManager(){
        return mOgreMeshManager;
    }

    std::shared_ptr<Rect2dManager> BaseSingleton::getRect2dManager(){
        return mRect2dManager;
    }

    std::shared_ptr<ValueRegistry> BaseSingleton::getGlobalRegistry(){
        return mGlobalRegistry;
    }

    std::shared_ptr<EntityManager> BaseSingleton::getEntityManager(){
        return mEntityManager;
    }

    std::shared_ptr<PhysicsManager> BaseSingleton::getPhysicsManager(){
        return mPhysicsManager;
    }


    std::shared_ptr<InputManager> BaseSingleton::getInputManager(){
        return mInputManager;
    }

    std::shared_ptr<GuiManager> BaseSingleton::getGuiManager(){
        return mGuiManager;
    }

    std::shared_ptr<GuiInputProcessor> BaseSingleton::getGuiInputProcessor(){
        return mGuiInputProcessor;
    }

    std::shared_ptr<ScriptManager> BaseSingleton::getScriptManager(){
        return mScriptManager;
    }


    std::shared_ptr<AnimationManager> BaseSingleton::getAnimationManager(){
        return mAnimationManager;
    }

    std::shared_ptr<AudioManager> BaseSingleton::getAudioManager(){
        return mAudioManager;
    }

    Window* BaseSingleton::getWindow(){
        return mWindow;
    }

    Base* BaseSingleton::getBase(){
        return mBase;
    }

    Ogre::SceneManager* BaseSingleton::getSceneManager(){
        return mSceneManager;
    }

    const PerformanceStats& BaseSingleton::getPerformanceStats(){
        return mPerformanceStats;
    }
}
