#pragma once

#include "ScriptNamespace.h"
#include "World/Entity/eId.h"

namespace AV{
    struct squirrelEIdData : public squirrelDataType{
        squirrelEIdData(eId id) : squirrelDataType(AV_SQ_DATA_TYPE_EID), id(id) {};
        ~squirrelEIdData(){ }

        //The stored eId.
        eId id;
    };

    class EntityNamespace : public ScriptNamespace{
    public:
        EntityNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger setEntityPosition(HSQUIRRELVM vm);
        static SQInteger createEntity(HSQUIRRELVM vm);

        static void _wrapEID(HSQUIRRELVM vm, eId entity);
    };
}
