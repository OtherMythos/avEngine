#pragma once

#include <memory>

namespace AV{
    class Base;
    class ScriptingStateManager;
    class SerialisationManager;
    class OgreMeshManager;
    class MovableTextureManager;

    class BaseSingleton{
        friend class Base;
    public:
        static std::shared_ptr<ScriptingStateManager> getScriptedStateManager();
        static std::shared_ptr<SerialisationManager> getSerialisationManager();
        static std::shared_ptr<OgreMeshManager> getOgreMeshManager();
        static std::shared_ptr<MovableTextureManager> getMovableTextureManager();

    private:
        static void initialise(
            std::shared_ptr<ScriptingStateManager> scriptedStateManager,
            std::shared_ptr<SerialisationManager> serialisationManager,
            std::shared_ptr<OgreMeshManager> ogreMeshManager,
            std::shared_ptr<MovableTextureManager> movableTextureManager
        );

        static std::shared_ptr<ScriptingStateManager> mScriptedStateManager;
        static std::shared_ptr<SerialisationManager> mSerialisationManager;
        static std::shared_ptr<OgreMeshManager> mOgreMeshManager;
        static std::shared_ptr<MovableTextureManager> mMovableTextureManager;
    };
}
