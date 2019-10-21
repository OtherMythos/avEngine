#include "BaseSingleton.h"

namespace AV{
    std::shared_ptr<ScriptingStateManager> BaseSingleton::mScriptedStateManager;
    std::shared_ptr<SerialisationManager> BaseSingleton::mSerialisationManager;
    std::shared_ptr<OgreMeshManager> BaseSingleton::mOgreMeshManager;
    std::shared_ptr<MovableTextureManager> BaseSingleton::mMovableTextureManager;

    void BaseSingleton::initialise(
        std::shared_ptr<ScriptingStateManager> scriptedStateManager,
        std::shared_ptr<SerialisationManager> serialisationManager,
        std::shared_ptr<OgreMeshManager> ogreMeshManager,
        std::shared_ptr<MovableTextureManager> movableTextureManager
    ){

        mScriptedStateManager = scriptedStateManager;
        mSerialisationManager = serialisationManager;
        mOgreMeshManager = ogreMeshManager;
        mMovableTextureManager = movableTextureManager;
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

    std::shared_ptr<MovableTextureManager> BaseSingleton::getMovableTextureManager(){
        return mMovableTextureManager;
    }
}
