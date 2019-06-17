#pragma once

#include <squirrel.h>
#include "World/Entity/eId.h"
#include "Scripting/ScriptDataPacker.h"

namespace AV{
    class EntityClass{
    public:
        EntityClass() { };

        void setupClass(HSQUIRRELVM vm);

        static void _entityClassFromEID(HSQUIRRELVM vm, eId entity);
        static SQObject _objFromEID(HSQUIRRELVM vm, eId entity);

        static eId getEID(HSQUIRRELVM vm, int stackIndex);

        /**
        Set the tracked eid of this entity to be invalid.
        This function expects an entity instance to be on the top of the stack.
        */
        static void invalidateEntityInstance(HSQUIRRELVM vm);

    private:
        static SQInteger setEntityPosition(HSQUIRRELVM vm);
        static SQInteger getEntityPosition(HSQUIRRELVM vm);
        static SQInteger moveEntity(HSQUIRRELVM vm);
        static SQInteger checkValid(HSQUIRRELVM vm);
        static SQInteger checkTrackable(HSQUIRRELVM vm);
        static SQInteger isTracked(HSQUIRRELVM vm);
        static SQInteger _entityCompare(HSQUIRRELVM vm);

        static SQInteger EIDReleaseHook(SQUserPointer p,SQInteger size);

        static SQObject classObject;

        static ScriptDataPacker<uint64_t> eIdData;
    };
}
