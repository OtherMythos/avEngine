#include "BaseSingleton.h"

namespace AV{
    std::shared_ptr<ScriptingStateManager> BaseSingleton::mScriptedStateManager;
    std::shared_ptr<SerialisationManager> BaseSingleton::mSerialisationManager;
    std::shared_ptr<PhysicsShapeManager> BaseSingleton::mPhysicsShapeManager;
    std::shared_ptr<PhysicsBodyConstructor> BaseSingleton::mPhysicsBodyConstructor;
    std::shared_ptr<PhysicsBodyDestructor> BaseSingleton::mPhysicsBodyDestructor;
    std::shared_ptr<OgreMeshManager> BaseSingleton::mOgreMeshManager;

    void BaseSingleton::initialise(
        std::shared_ptr<ScriptingStateManager> scriptedStateManager,
        std::shared_ptr<SerialisationManager> serialisationManager,
        std::shared_ptr<PhysicsShapeManager> physicsShapeManager,
        std::shared_ptr<PhysicsBodyConstructor> physicsBodyConstructor,
        std::shared_ptr<PhysicsBodyDestructor> physicsBodyDestructor,
        std::shared_ptr<OgreMeshManager> ogreMeshManager
    ){

        mScriptedStateManager = scriptedStateManager;
        mSerialisationManager = serialisationManager;
        mPhysicsShapeManager = physicsShapeManager;
        mPhysicsBodyConstructor = physicsBodyConstructor;
        mPhysicsBodyDestructor = physicsBodyDestructor;
        mOgreMeshManager = ogreMeshManager;
    }

    std::shared_ptr<ScriptingStateManager> BaseSingleton::getScriptedStateManager(){
        return mScriptedStateManager;
    }

    std::shared_ptr<SerialisationManager> BaseSingleton::getSerialisationManager(){
        return mSerialisationManager;
    }

    std::shared_ptr<PhysicsShapeManager> BaseSingleton::getPhysicsShapeManager(){
        return mPhysicsShapeManager;
    }

    std::shared_ptr<PhysicsBodyConstructor> BaseSingleton::getPhysicsBodyConstructor(){
        return mPhysicsBodyConstructor;
    }

    std::shared_ptr<PhysicsBodyDestructor> BaseSingleton::getPhysicsBodyDestructor(){
        return mPhysicsBodyDestructor;
    }

    std::shared_ptr<OgreMeshManager> BaseSingleton::getOgreMeshManager(){
        return mOgreMeshManager;
    }
}
