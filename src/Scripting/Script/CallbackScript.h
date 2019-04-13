#pragma once

#include "BaseScript.h"
#include <map>

namespace AV{
    class CallbackScript : public BaseScript{
    public:
        CallbackScript(HSQUIRRELVM vm);
        CallbackScript();
        ~CallbackScript();

        bool prepare(const Ogre::String& path);
        void initialise(HSQUIRRELVM vm);

        bool call(const Ogre::String& functionName);

        void release();

    private:
        HSQUIRRELVM mVm;
        HSQOBJECT mMainClosure;
        HSQOBJECT mMainTable;

        bool _compileMainClosure(const Ogre::String& path);
        bool _createMainTable();
        bool _callMainClosure();
        bool _parseClosureTable();

        bool mPrepared = false;
        bool mInitialised = false;

        std::map<Ogre::String, HSQOBJECT> mClosureMap;
    };
}
