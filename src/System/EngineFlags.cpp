#include "EngineFlags.h"

namespace AV{
    /**
    When this flag is true, functions such as set position on various items should not be allowed.
    This is in order to preserve the state of the scene.
    */
    bool EngineFlags::mSceneClean = false;
}
