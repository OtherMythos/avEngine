#pragma once

#include <squirrel.h>

namespace AV{
    class ScriptNamespace{
    public:
        ScriptNamespace() {};

        virtual void setupNamespace(HSQUIRRELVM vm) = 0;

    protected:
        /**
         Add a function to the squirrel vm.
         
         @arg v
         The target VM.
         @arg f
         A function pointer to the c++ function which is to be exposed to squirrel.
         @arg fname
         The name the function should use in squirrel.
         @arg numParams
         The number of parameters this function should check for. Add 1 to your intended ammount to account for squirrel passing in the invisible parameter 'this'.
         @arg typeMask
         The typemask to use. Add an . at the start to account for the invisible parameter.
         
         @remarks
         Squirrel seems to have an invisible variable passed into each function.
         So say I wanted to force parameters such as three numbers, which would be nnn, I would actually need to put .nnn .
         The . at the front would account for the invisible parameter. You would also need to increment the number by 1 to account for this extra parameter.
         */
        void _addFunction(HSQUIRRELVM v, SQFUNCTION f, const char *fname, int numParams = 0, const char *typeMask = "");
    };
}
