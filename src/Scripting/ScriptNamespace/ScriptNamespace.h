#pragma once

#include <squirrel.h>
#include <map>

namespace AV{

    enum AV_SQ_DATA_TYPE{
        AV_SQ_DATA_TYPE_NONE,
        AV_SQ_DATA_TYPE_MESH
    };

    struct squirrelDataType{
        squirrelDataType(AV_SQ_DATA_TYPE type)
        : type(type) {}

        AV_SQ_DATA_TYPE type = AV_SQ_DATA_TYPE_NONE;
    };

    class ScriptNamespace{
    public:
        ScriptNamespace() {};

        virtual void setupNamespace(HSQUIRRELVM vm) = 0;

    protected:
        /**
        A struct to contain information about how a testing function should be bound.

        @arg typeMask
        The typemask of the function. This is a plain squirrel type mask, for instance .iii for a function which takes three integers.
        Remember to include the . for the invisible 'this' variable.
        @arg typeCount
        How many parameters the type mask contains. Remember to add one for the invisible 'this' variable.
        @arg function
        The actual function to be exposed.
        */
        struct TestFunctionEntry{
            const char* typeMask;
            int typeCount;
            SQFUNCTION function;
        };

        typedef std::map<const char*, TestFunctionEntry> RedirectFunctionMap;

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

        /**
        Redirect the bindings of a function based on the redirect flag.
        This function can be used to expose c++ functions, or redirect all the exposures to a single function.
        This is done depending on the redirect flag.

        @remarks
        This function is used by the test namespace to re-direct the bindings of functions based on whether test mode is enabled or disabled.
        The details of the function to be exposed are described by the map.
        If the redirect parameter is true the function will be exposed as the redirectFunction rather than the function described in the map.
        */
        void _redirectFunctionMap(HSQUIRRELVM v, SQFUNCTION redirectFunction, const RedirectFunctionMap &rMap, bool redirect);
    };
}
