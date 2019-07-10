#pragma once

#include <memory>

namespace AV{
    class Base;
    class ScriptingStateManager;
    class SerialisationManager;
    class OgreMeshManager;

    class BaseSingleton{
        friend class Base;
    public:
        static std::shared_ptr<ScriptingStateManager> getScriptedStateManager();
        static std::shared_ptr<SerialisationManager> getSerialisationManager();
        static std::shared_ptr<OgreMeshManager> getOgreMeshManager();

    private:
        static void initialise(
            std::shared_ptr<ScriptingStateManager> scriptedStateManager,
            std::shared_ptr<SerialisationManager> serialisationManager,
            std::shared_ptr<OgreMeshManager> ogreMeshManager
        );

        static std::shared_ptr<ScriptingStateManager> mScriptedStateManager;
        static std::shared_ptr<SerialisationManager> mSerialisationManager;
        static std::shared_ptr<OgreMeshManager> mOgreMeshManager;
    };
}
