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
    class ScriptManager;
    class DebugDrawer;

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
        static std::shared_ptr<GuiManager> getGuiManager();
        static std::shared_ptr<ScriptManager> getScriptManager();
        static Window* getWindow();

    private:
        static void initialise(
            Window* window,
            std::shared_ptr<ScriptingStateManager> scriptedStateManager,
            std::shared_ptr<SerialisationManager> serialisationManager,
            std::shared_ptr<OgreMeshManager> ogreMeshManager,
            std::shared_ptr<Rect2dManager> rect2dManager,
            std::shared_ptr<DialogManager> dialogManager,
            std::shared_ptr<ValueRegistry> valueRegistry,
            std::shared_ptr<TerrainManager> terrainManager,
            std::shared_ptr<InputManager> inputManager,
            std::shared_ptr<GuiManager> guiManager,
            std::shared_ptr<ScriptManager> scriptManager
        );

        static std::shared_ptr<ScriptingStateManager> mScriptedStateManager;
        static std::shared_ptr<SerialisationManager> mSerialisationManager;
        static std::shared_ptr<OgreMeshManager> mOgreMeshManager;
        static std::shared_ptr<Rect2dManager> mRect2dManager;
        static std::shared_ptr<DialogManager> mDialogManager;
        static std::shared_ptr<ValueRegistry> mGlobalRegistry;
        static std::shared_ptr<TerrainManager> mTerrainManager;
        static std::shared_ptr<InputManager> mInputManager;
        static std::shared_ptr<GuiManager> mGuiManager;
        static std::shared_ptr<ScriptManager> mScriptManager;
        static Window* mWindow;

    #ifdef DEBUGGING_TOOLS
        static DebugDrawer* mDebugDrawer;

    public:
        static void setupDebuggerTools(DebugDrawer* drawer);
        static DebugDrawer* getDebugDrawer();
    #endif
    };
}
