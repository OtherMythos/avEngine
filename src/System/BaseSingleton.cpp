#include "BaseSingleton.h"

namespace AV{
    std::shared_ptr<ScriptingStateManager> BaseSingleton::mScriptedStateManager;
    std::shared_ptr<SerialisationManager> BaseSingleton::mSerialisationManager;
    std::shared_ptr<OgreMeshManager> BaseSingleton::mOgreMeshManager;
    std::shared_ptr<Rect2dManager> BaseSingleton::mRect2dManager;
    std::shared_ptr<DialogManager> BaseSingleton::mDialogManager;
    std::shared_ptr<ValueRegistry> BaseSingleton::mGlobalRegistry;
    std::shared_ptr<TerrainManager> BaseSingleton::mTerrainManager;
    std::shared_ptr<InputManager> BaseSingleton::mInputManager;
    std::shared_ptr<TimerManager> BaseSingleton::mTimerManager;
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

        void BaseSingleton::setupDebuggerTools(
            DebugDrawer* debugDrawer
        ){
            mDebugDrawer = debugDrawer;
        }

        DebugDrawer* BaseSingleton::getDebugDrawer(){
            return mDebugDrawer;
        }
    #endif

    void BaseSingleton::initialise(
        Base* base,
        Window* window,
        std::shared_ptr<ScriptingStateManager> scriptedStateManager,
        std::shared_ptr<SerialisationManager> serialisationManager,
        std::shared_ptr<OgreMeshManager> ogreMeshManager,
        std::shared_ptr<Rect2dManager> rect2dManager,
        std::shared_ptr<DialogManager> dialogManager,
        std::shared_ptr<ValueRegistry> valueRegistry,
        std::shared_ptr<TerrainManager> terrainManager,
        std::shared_ptr<InputManager> inputManager,
        std::shared_ptr<TimerManager> timerManager,
        std::shared_ptr<GuiManager> guiManager,
        std::shared_ptr<GuiInputProcessor> guiInputProcessor,
        std::shared_ptr<ScriptManager> scriptManager,
        std::shared_ptr<AnimationManager> animationManager,
        std::shared_ptr<AudioManager> audioManager
    ){

        mWindow = window;
        mBase = base;
        mScriptedStateManager = scriptedStateManager;
        mSerialisationManager = serialisationManager;
        mOgreMeshManager = ogreMeshManager;
        mRect2dManager = rect2dManager;
        mDialogManager = dialogManager;
        mGlobalRegistry = valueRegistry;
        mTerrainManager = terrainManager;
        mInputManager = inputManager;
        mGuiManager = guiManager;
        mGuiInputProcessor = guiInputProcessor;
        mScriptManager = scriptManager;
        mTimerManager = timerManager;
        mAnimationManager = animationManager;
        mAudioManager = audioManager;
    }

    std::shared_ptr<ScriptingStateManager> BaseSingleton::getScriptedStateManager(){
        return mScriptedStateManager;
    }

    std::shared_ptr<SerialisationManager> BaseSingleton::getSerialisationManager(){
        return mSerialisationManager;
    }

    std::shared_ptr<OgreMeshManager> BaseSingleton::getOgreMeshManager(){
        return mOgreMeshManager;
    }

    std::shared_ptr<Rect2dManager> BaseSingleton::getRect2dManager(){
        return mRect2dManager;
    }

    std::shared_ptr<DialogManager> BaseSingleton::getDialogManager(){
        return mDialogManager;
    }

    std::shared_ptr<ValueRegistry> BaseSingleton::getGlobalRegistry(){
        return mGlobalRegistry;
    }

    std::shared_ptr<TerrainManager> BaseSingleton::getTerrainManager(){
        return mTerrainManager;
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

    std::shared_ptr<TimerManager> BaseSingleton::getTimerManager(){
        return mTimerManager;
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
