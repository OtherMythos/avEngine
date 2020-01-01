#include "BaseSingleton.h"

namespace AV{
    std::shared_ptr<ScriptingStateManager> BaseSingleton::mScriptedStateManager;
    std::shared_ptr<SerialisationManager> BaseSingleton::mSerialisationManager;
    std::shared_ptr<OgreMeshManager> BaseSingleton::mOgreMeshManager;
    std::shared_ptr<MovableTextureManager> BaseSingleton::mMovableTextureManager;
    std::shared_ptr<Rect2dManager> BaseSingleton::mRect2dManager;
    std::shared_ptr<DialogManager> BaseSingleton::mDialogManager;
    Window* BaseSingleton::mWindow = 0;

    void BaseSingleton::initialise(
        Window* window,
        std::shared_ptr<ScriptingStateManager> scriptedStateManager,
        std::shared_ptr<SerialisationManager> serialisationManager,
        std::shared_ptr<OgreMeshManager> ogreMeshManager,
        std::shared_ptr<MovableTextureManager> movableTextureManager,
        std::shared_ptr<Rect2dManager> rect2dManager,
        std::shared_ptr<DialogManager> dialogManager
    ){

        mWindow = window;
        mScriptedStateManager = scriptedStateManager;
        mSerialisationManager = serialisationManager;
        mOgreMeshManager = ogreMeshManager;
        mMovableTextureManager = movableTextureManager;
        mRect2dManager = rect2dManager;
        mDialogManager = dialogManager;
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

    std::shared_ptr<Rect2dManager> BaseSingleton::getRect2dManager(){
        return mRect2dManager;
    }

    std::shared_ptr<DialogManager> BaseSingleton::getDialogManager(){
        return mDialogManager;
    }

    Window* BaseSingleton::getWindow(){
        return mWindow;
    }
}
