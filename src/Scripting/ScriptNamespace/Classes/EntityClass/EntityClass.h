#pragma once

#include <squirrel.h>

namespace AV{
    class EntityClass{
    public:
        EntityClass() { };

        void setupClass(HSQUIRRELVM vm);

    private:
        static SQInteger setEntityPosition(HSQUIRRELVM vm);
        static SQInteger checkValid(HSQUIRRELVM vm);
        static SQInteger checkTrackable(HSQUIRRELVM vm);
        
        static SQInteger EIDReleaseHook(SQUserPointer p,SQInteger size);
    };
}
