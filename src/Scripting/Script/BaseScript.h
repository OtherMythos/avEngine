#pragma once

#include <squirrel.h>
#include "OgreString.h"

namespace AV{
    class BaseScript{
    public:
        BaseScript();
        virtual ~BaseScript();

        Ogre::String getFilePath() { return filePath; }

    protected:
        virtual void _processSquirrelFailure(HSQUIRRELVM vm);
        

        Ogre::String filePath;
    };
}
