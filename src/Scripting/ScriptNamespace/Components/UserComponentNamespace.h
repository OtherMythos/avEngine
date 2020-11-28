#pragma once

#include "squirrel.h"
#include "System/EnginePrerequisites.h"
#include "World/Entity/UserComponents/UserComponentData.h"

namespace AV{
    class UserComponentNamespace{
    public:
        UserComponentNamespace() {}

        void setupNamespace(HSQUIRRELVM vm);

        template <ComponentType A>
        static SQInteger add(HSQUIRRELVM vm);
        template <ComponentType A>
        static SQInteger remove(HSQUIRRELVM v);
        template <ComponentType A>
        static SQInteger set(HSQUIRRELVM vm);
        template <ComponentType A>
        static SQInteger get(HSQUIRRELVM vm);

    private:
        static SQInteger _add(HSQUIRRELVM v, ComponentType i);
        static SQInteger _remove(HSQUIRRELVM vm, ComponentType i);
        static SQInteger _set(HSQUIRRELVM vm, ComponentType i);
        static SQInteger _get(HSQUIRRELVM vm, ComponentType i);

        static SQInteger userComponentGetMetamethod(HSQUIRRELVM vm);
    };
}

