#pragma once

#include <memory>

namespace AV{
    class Base;
    class ScriptingStateManager;
    class SerialisationManager;
    class OgreMeshManager;
    class MovableTextureManager;
    class Rect2dManager;
    class DialogManager;
    class Window;

    class BaseSingleton{
        friend class Base;
    public:
        static std::shared_ptr<ScriptingStateManager> getScriptedStateManager();
        static std::shared_ptr<SerialisationManager> getSerialisationManager();
        static std::shared_ptr<OgreMeshManager> getOgreMeshManager();
        static std::shared_ptr<MovableTextureManager> getMovableTextureManager();
        static std::shared_ptr<Rect2dManager> getRect2dManager();
        static std::shared_ptr<DialogManager> getDialogManager();
        static Window* getWindow();

    private:
        static void initialise(
            Window* window,
            std::shared_ptr<ScriptingStateManager> scriptedStateManager,
            std::shared_ptr<SerialisationManager> serialisationManager,
            std::shared_ptr<OgreMeshManager> ogreMeshManager,
            std::shared_ptr<MovableTextureManager> movableTextureManager,
            std::shared_ptr<Rect2dManager> rect2dManager,
            std::shared_ptr<DialogManager> getDialogManager
        );

        static std::shared_ptr<ScriptingStateManager> mScriptedStateManager;
        static std::shared_ptr<SerialisationManager> mSerialisationManager;
        static std::shared_ptr<OgreMeshManager> mOgreMeshManager;
        static std::shared_ptr<MovableTextureManager> mMovableTextureManager;
        static std::shared_ptr<Rect2dManager> mRect2dManager;
        static std::shared_ptr<DialogManager> mDialogManager;
        static Window* mWindow;
    };
}
