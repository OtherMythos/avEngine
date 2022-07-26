#include "UserComponentNamespace.h"

#include "World/Entity/Logic/ScriptComponentLogic.h"
#include "Scripting/ScriptNamespace/Classes/Entity/EntityUserData.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "World/Entity/UserComponents/UserComponentManager.h"
#include "World/Entity/Logic/UserComponentLogic.h"

#include "System/SystemSetup/SystemSettings.h"

namespace AV{

    SQObject userComponentTables[NUM_USER_COMPONENTS];

    #define CHECK_ERROR(xx) \
        SQInteger __result__ = xx; \
        if(SQ_FAILED(__result__)) return __result__;

    inline SQInteger _checkError(HSQUIRRELVM vm, UserComponentLogic::ErrorTypes e){
        switch(e){
            default:
            case UserComponentLogic::SUCCESS: return 0;
            case UserComponentLogic::NO_COMPONENT: return sq_throwerror(vm, "Entity does not have this component.");
            case UserComponentLogic::COMPONENT_NOT_POPULATED: return sq_throwerror(vm, "No implementation of that component was provided.");
        }
    }

    SQInteger UserComponentNamespace::_add(HSQUIRRELVM vm, ComponentType i){
        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, -1, &id));

        CHECK_ERROR(_checkError(vm, UserComponentLogic::add(id, i)));

        return 0;
    }

    SQInteger UserComponentNamespace::_remove(HSQUIRRELVM vm, ComponentType i){
        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, -1, &id));

        CHECK_ERROR(_checkError(vm, UserComponentLogic::remove(id, i)));

        return 0;
    }

    bool _squirrelTypeMatchesComponentType(SQObjectType t, ComponentDataTypes c){
        switch(t){
            case OT_INTEGER: return c == ComponentDataTypes::INT;
            case OT_FLOAT: return c == ComponentDataTypes::FLOAT;
            case OT_BOOL: return c == ComponentDataTypes::BOOL;
            default:
                return false;
        }
    }

    SQInteger UserComponentNamespace::_set(HSQUIRRELVM vm, ComponentType i){
        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, 2, &id));

        SQInteger varId;
        sq_getinteger(vm, 3, &varId);
        const UserComponentSettings::ComponentSetting& settings = SystemSettings::getUserComponentSettings().vars[i];
        if(varId < 0 || varId > settings.numVars) return sq_throwerror(vm, "Invalid variable id");

        ComponentDataTypes variableType = getTypeOfVariable(settings.componentVars, varId);
        if(variableType == ComponentDataTypes::NONE) return sq_throwerror(vm, "Component does not use this variable.");

        SQObjectType type = sq_gettype(vm, 4);

        bool matches = _squirrelTypeMatchesComponentType(type, variableType);
        if(!matches) return sq_throwerror(vm, "Invalid type passed for user component variable.");

        UserComponentDataEntry compData;
        switch(type){
            case OT_INTEGER:{
                SQInteger val;
                sq_getinteger(vm, 4, &val);
                compData.i = val;
                break;
            }
            case OT_FLOAT:{
                SQFloat val;
                sq_getfloat(vm, 4, &val);
                compData.f = val;
                break;
            }
            case OT_BOOL:{
                SQBool val;
                sq_getbool(vm, 4, &val);
                compData.b = val;
                break;
            }
            default:{
                assert(false);
                break;
            }
        }

        CHECK_ERROR(_checkError(vm, UserComponentLogic::set(id, static_cast<ComponentType>(i), varId, compData)));

        return 0;
    }

    SQInteger UserComponentNamespace::_get(HSQUIRRELVM vm, ComponentType i){
        //TODO reduce duplication with the above function.
        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, 2, &id));
        SQInteger varId;
        sq_getinteger(vm, 3, &varId);

        const UserComponentSettings::ComponentSetting& settings = SystemSettings::getUserComponentSettings().vars[i];
        if(varId < 0 || varId >= settings.numVars) return sq_throwerror(vm, "Invalid variable id");

        ComponentDataTypes type = getTypeOfVariable(settings.componentVars, varId);
        if(type == ComponentDataTypes::NONE) return sq_throwerror(vm, "Component does not use this variable.");

        UserComponentDataEntry compData;
        CHECK_ERROR(_checkError(vm, UserComponentLogic::get(id, static_cast<ComponentType>(i), varId, &compData)));

        switch(type){
            case ComponentDataTypes::INT:{
                sq_pushinteger(vm, compData.i);
                break;
            }
            case ComponentDataTypes::FLOAT:{
                sq_pushfloat(vm, compData.f);
                break;
            }
            case ComponentDataTypes::BOOL:{
                sq_pushbool(vm, compData.b);
                break;
            }
            default:{
                assert(false);
                break;
            }
        }

        return 1;
    }

    template <ComponentType A>
    SQInteger UserComponentNamespace::remove(HSQUIRRELVM vm){
        return _remove(vm, A);
    }

    template <ComponentType A>
    SQInteger UserComponentNamespace::add(HSQUIRRELVM vm){
        return _add(vm, A);
    }

    template <ComponentType A>
    SQInteger UserComponentNamespace::set(HSQUIRRELVM vm){
        return _set(vm, A);
    }

    template <ComponentType A>
    SQInteger UserComponentNamespace::get(HSQUIRRELVM vm){
        return _get(vm, A);
    }


    SQInteger UserComponentNamespace::userComponentGetMetamethod(HSQUIRRELVM vm){
        SQObjectType t = sq_gettype(vm, -1);
        if(t != OT_INTEGER) return sq_throwerror(vm, "An integer should be passed to reference a user component.");

        SQInteger i;
        sq_getinteger(vm, -1, &i);

        uint8 numReigstered = SystemSettings::getUserComponentSettings().numRegisteredComponents;
        if(i < 0 || i >= numReigstered) return sq_throwerror(vm, "Invalid user component id.");

        sq_pushobject(vm, userComponentTables[i]);

        return 1;
    }

    void UserComponentNamespace::setupNamespace(HSQUIRRELVM vm){

        sq_pushstring(vm, _SC("user"), -1);

        //Create an empty table, and assign another to it with the delegate table, containing the getter metamethod.
        sq_newtable(vm);

        {
            //Create the delegate table
            sq_newtableex(vm, 1);
            ScriptUtils::addFunction(vm, userComponentGetMetamethod, "_get");
            bool result = SQ_SUCCEEDED(sq_setdelegate(vm, -2));
            assert(result);
        }

        sq_newslot(vm, -3, false);


        #define COMP_FUNCTIONS(nn) \
            add<nn>, \
            remove<nn>, \
            set<nn>, \
            get<nn>

        SQFUNCTION functions0[] = { COMP_FUNCTIONS(0) };
        SQFUNCTION functions1[] = { COMP_FUNCTIONS(1) };
        SQFUNCTION functions2[] = { COMP_FUNCTIONS(2) };
        SQFUNCTION functions3[] = { COMP_FUNCTIONS(3) };
        SQFUNCTION functions4[] = { COMP_FUNCTIONS(4) };
        SQFUNCTION functions5[] = { COMP_FUNCTIONS(5) };
        SQFUNCTION functions6[] = { COMP_FUNCTIONS(6) };
        SQFUNCTION functions7[] = { COMP_FUNCTIONS(7) };
        SQFUNCTION functions8[] = { COMP_FUNCTIONS(8) };
        SQFUNCTION functions9[] = { COMP_FUNCTIONS(9) };
        SQFUNCTION functions10[] = { COMP_FUNCTIONS(10) };
        SQFUNCTION functions11[] = { COMP_FUNCTIONS(11) };
        SQFUNCTION functions12[] = { COMP_FUNCTIONS(12) };
        SQFUNCTION functions13[] = { COMP_FUNCTIONS(13) };
        SQFUNCTION functions14[] = { COMP_FUNCTIONS(14) };
        SQFUNCTION functions15[] = { COMP_FUNCTIONS(15) };
        //static_assert(MAX_COLLISION_WORLDS == 16, "Update the above code if changing the number of collision worlds.");


        SQFUNCTION* functions[] = {
            &(functions0[0]), &(functions1[0]), &(functions2[0]), &(functions3[0]),
            &(functions4[0]), &(functions5[0]), &(functions6[0]), &(functions7[0]),
            &(functions8[0]), &(functions9[0]), &(functions10[0]), &(functions11[0]),
            &(functions12[0]), &(functions13[0]), &(functions14[0]), &(functions15[0]),
        };
        //Create each collision world object for the array.
        //These are later returned as part of the metamethod.
        uint8 numReigstered = SystemSettings::getUserComponentSettings().numRegisteredComponents;
        for(int i = 0; i < numReigstered; i++){
            sq_newtable(vm);

            ScriptUtils::addFunction(vm, (*(functions[i]+0)), "add", 2, ".u");
            ScriptUtils::addFunction(vm, (*(functions[i]+1)), "remove", 2, ".u");
            ScriptUtils::addFunction(vm, (*(functions[i]+2)), "set", 4, ".uii|f|b");
            ScriptUtils::addFunction(vm, (*(functions[i]+3)), "get", 3, ".ui");

            sq_resetobject( &(userComponentTables[i]) );
            sq_getstackobj(vm, -1,  &(userComponentTables[i]) );
            sq_addref(vm,  &(userComponentTables[i]) );
            sq_pop(vm, 1);
        }

        #undef COMP_FUNCTIONS

    }
}
