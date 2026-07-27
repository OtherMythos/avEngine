#pragma once

#include "ComponentLogic.h"

#include "OgreString.h"

namespace AV{

    enum class ScriptComponentAddResult{
        SUCCESS,
        ALREADY_HAS_COMPONENT,
        FAILURE
    };

    class ScriptComponentLogic : public ComponentLogic{
    public:
        static ScriptComponentAddResult add(eId id, const Ogre::String &scriptPath);
        static bool remove(eId id);

    };
}
