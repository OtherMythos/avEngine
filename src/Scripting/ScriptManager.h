#pragma once

#include <squirrel.h>

namespace AV {
    class ScriptManager{
    public:
        /**
         Call to initialise the Script Manager. This should be done at startup.
         */
        static void initialise();
        
        /**
         Run a single script.
         
         @arg scriptPath
         The path of the target script.
         */
        static void runScript(const char *scriptPath);
        
    private:
        /**
         The Squirrel Virtual Machine used for running scripts.
         */
        static HSQUIRRELVM _sqvm;
        
        /**
         Internally setup the class.
         */
        static void _setupVM(HSQUIRRELVM vm);
    };
}
