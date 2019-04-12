#pragma once

#include "OgreString.h"
#include <squirrel.h>
#include <map>

namespace AV{
    class CallbackScript{
    public:
        CallbackScript(HSQUIRRELVM vm);

        bool prepare(const Ogre::String& path);

        bool call(const Ogre::String& functionName);

        void destroy();

    private:
        HSQUIRRELVM mVm;
        HSQOBJECT mMainClosure;
        HSQOBJECT mMainTable;

        bool _compileMainClosure(const Ogre::String& path);
        bool _createMainTable();
        bool _callMainClosure();
        bool _parseClosureTable();

        bool mPrepared = false;

        std::map<Ogre::String, HSQOBJECT> mClosureMap;
    };
}
