#pragma once

#include <squirrel.h>
#include "OgreString.h"

namespace AV{
    class BaseScript{
    public:
        BaseScript();
        ~BaseScript();

    protected:
        virtual void _processSquirrelFailure(HSQUIRRELVM vm);

        Ogre::String filePath;
    };
}
