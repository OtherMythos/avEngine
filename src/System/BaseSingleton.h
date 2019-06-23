#pragma once

#include <memory>

namespace AV{
    class Base;
    class ScriptingStateManager;
    class SerialisationManager;
    class PhysicsShapeManager;
    class PhysicsBodyConstructor;
    class OgreMeshManager;

    class BaseSingleton{
        friend class Base;
    public:
        static std::shared_ptr<ScriptingStateManager> getScriptedStateManager();
        static std::shared_ptr<SerialisationManager> getSerialisationManager();
        static std::shared_ptr<PhysicsShapeManager> getPhysicsShapeManager();
        static std::shared_ptr<PhysicsBodyConstructor> getPhysicsBodyConstructor();
        static std::shared_ptr<OgreMeshManager> getOgreMeshManager();

    private:
        static void initialise(
            std::shared_ptr<ScriptingStateManager> scriptedStateManager,
            std::shared_ptr<SerialisationManager> serialisationManager,
            std::shared_ptr<PhysicsShapeManager> physicsShapeManager,
            std::shared_ptr<PhysicsBodyConstructor> physicsBodyConstructor,
            std::shared_ptr<OgreMeshManager> ogreMeshManager
        );

        static std::shared_ptr<ScriptingStateManager> mScriptedStateManager;
        static std::shared_ptr<SerialisationManager> mSerialisationManager;
        static std::shared_ptr<PhysicsShapeManager> mPhysicsShapeManager;
        static std::shared_ptr<PhysicsBodyConstructor> mPhysicsBodyConstructor;
        static std::shared_ptr<OgreMeshManager> mOgreMeshManager;
    };
}
