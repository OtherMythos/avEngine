#include "BaseSingleton.h"

namespace AV{
    std::shared_ptr<ScriptingStateManager> BaseSingleton::mScriptedStateManager;
    std::shared_ptr<SerialisationManager> BaseSingleton::mSerialisationManager;
    std::shared_ptr<OgreMeshManager> BaseSingleton::mOgreMeshManager;
    std::shared_ptr<Rect2dManager> BaseSingleton::mRect2dManager;
    std::shared_ptr<DialogManager> BaseSingleton::mDialogManager;
    std::shared_ptr<ValueRegistry> BaseSingleton::mGlobalRegistry;
    std::shared_ptr<TerrainManager> BaseSingleton::mTerrainManager;
    Window* BaseSingleton::mWindow = 0;

    void BaseSingleton::initialise(
        Window* window,
        std::shared_ptr<ScriptingStateManager> scriptedStateManager,
        std::shared_ptr<SerialisationManager> serialisationManager,
        std::shared_ptr<OgreMeshManager> ogreMeshManager,
        std::shared_ptr<Rect2dManager> rect2dManager,
        std::shared_ptr<DialogManager> dialogManager,
        std::shared_ptr<ValueRegistry> valueRegistry,
        std::shared_ptr<TerrainManager> terrainManager
    ){

        mWindow = window;
        mScriptedStateManager = scriptedStateManager;
        mSerialisationManager = serialisationManager;
        mOgreMeshManager = ogreMeshManager;
        mRect2dManager = rect2dManager;
        mDialogManager = dialogManager;
        mGlobalRegistry = valueRegistry;
        mTerrainManager = terrainManager;
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

    Window* BaseSingleton::getWindow(){
        return mWindow;
    }
}
