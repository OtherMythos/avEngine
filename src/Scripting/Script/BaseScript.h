#pragma once

#include <squirrel.h>
#include "OgreString.h"

namespace AV{

    /**
    Simple script base class.
    */
    class BaseScript{
    public:
        BaseScript();
        virtual ~BaseScript();

        Ogre::String getFilePath() { return mFilePath; }

    protected:
        virtual void _processSquirrelFailure(HSQUIRRELVM vm);


        Ogre::String mFilePath;
    };
}
