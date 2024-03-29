#pragma once

#include <memory>

namespace AV{
    class Base;
    class ScriptingStateManager;
    class SerialisationManager;
    class OgreMeshManager;
    class Rect2dManager;
    class DialogManager;
    class Window;
    class ValueRegistry;
    class TerrainManager;
    class InputManager;
    class GuiManager;
    class GuiInputProcessor;
    class ScriptManager;
    class DebugDrawer;
    class TimerManager;
    class AnimationManager;
    class AudioManager;

    struct PerformanceStats{
        float frameTime;
        float avgFPS;
        float fps;
    };

    class BaseSingleton{
        friend class Base;
    public:
        static std::shared_ptr<ScriptingStateManager> getScriptedStateManager();
        static std::shared_ptr<SerialisationManager> getSerialisationManager();
        static std::shared_ptr<OgreMeshManager> getOgreMeshManager();
        static std::shared_ptr<Rect2dManager> getRect2dManager();
        static std::shared_ptr<DialogManager> getDialogManager();
        static std::shared_ptr<ValueRegistry> getGlobalRegistry();
        static std::shared_ptr<TerrainManager> getTerrainManager();
        static std::shared_ptr<InputManager> getInputManager();
        static std::shared_ptr<TimerManager> getTimerManager();
        static std::shared_ptr<GuiManager> getGuiManager();
        static std::shared_ptr<GuiInputProcessor> getGuiInputProcessor();
        static std::shared_ptr<ScriptManager> getScriptManager();
        static std::shared_ptr<AnimationManager> getAnimationManager();
        static std::shared_ptr<AudioManager> getAudioManager();
        static Window* getWindow();
        static Base* getBase();

        static const PerformanceStats& getPerformanceStats();

    private:
        static void initialise(
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
            std::shared_ptr<AudioManager> getAudioManager
        );

        static std::shared_ptr<ScriptingStateManager> mScriptedStateManager;
        static std::shared_ptr<SerialisationManager> mSerialisationManager;
        static std::shared_ptr<OgreMeshManager> mOgreMeshManager;
        static std::shared_ptr<Rect2dManager> mRect2dManager;
        static std::shared_ptr<DialogManager> mDialogManager;
        static std::shared_ptr<ValueRegistry> mGlobalRegistry;
        static std::shared_ptr<TerrainManager> mTerrainManager;
        static std::shared_ptr<InputManager> mInputManager;
        static std::shared_ptr<TimerManager> mTimerManager;
        static std::shared_ptr<GuiManager> mGuiManager;
        static std::shared_ptr<GuiInputProcessor> mGuiInputProcessor;
        static std::shared_ptr<ScriptManager> mScriptManager;
        static std::shared_ptr<AnimationManager> mAnimationManager;
        static std::shared_ptr<AudioManager> mAudioManager;
        static Window* mWindow;
        static Base* mBase;

        static PerformanceStats mPerformanceStats;

    #ifdef DEBUGGING_TOOLS
        static DebugDrawer* mDebugDrawer;

    public:
        static void setupDebuggerTools(DebugDrawer* drawer);
        static DebugDrawer* getDebugDrawer();
    #endif
    };
}
