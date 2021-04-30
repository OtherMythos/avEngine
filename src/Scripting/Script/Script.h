#pragma once

#include "BaseScript.h"

namespace AV{
class ScriptVM;

/**
A class to simplify squirrel script compilation and execution.
Unlike callback scripts, this is just a single script which can be run as a whole.
*/
class Script : public BaseScript{
    public:
        Script(HSQUIRRELVM vm);
        Script();
        ~Script();

        bool compileFile(const SQChar *path);
        bool run();
        void release();

        void initialise(HSQUIRRELVM vm);

    private:
        HSQUIRRELVM vm;
        HSQOBJECT obj;

        bool available = false;
        bool initialised = false;
    };
}
