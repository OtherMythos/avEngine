#include "EngineFlags.h"

namespace AV{
    /**
    When this flag is true, functions such as set position on various items should not be allowed.
    This is in order to preserve the state of the scene.
    */
    bool EngineFlags::mSceneClean = false;

    //"Internal" is used by Ogre.
    const std::string EngineFlags::ENGINE_RES_PREREQUISITE = "avInternal";

    bool EngineFlags::resourceGroupValid(const std::string& resGroupName){
        return resGroupName.rfind(ENGINE_RES_PREREQUISITE, 0) != 0;
    }
}
