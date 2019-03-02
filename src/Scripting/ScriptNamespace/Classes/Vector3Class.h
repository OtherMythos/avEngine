#pragma once

#include <squirrel.h>

namespace AV{
    class Vector3Class{
    public:
        Vector3Class() { }

        static void setupClass(HSQUIRRELVM vm);

    private:
        static SQInteger vec3Constructor(HSQUIRRELVM vm);
        static SQInteger vec3AddMeta(HSQUIRRELVM vm);

        static SQMemberHandle handleX, handleY, handleZ;
    };
}
