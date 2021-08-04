#pragma once

#include <squirrel.h>

#include "System/SystemSetup/SystemSettings.h"
#include "World/Slot/ChunkCoordinate.h"
#include "World/Slot/SlotPosition.h"
#include "World/Entity/eId.h"
#include "Logger/Log.h"

#include <map>

#define SCRIPT_CHECK_WORLD() \
    World *world = WorldSingleton::getWorld(); \
    if(!world) return sq_throwerror(vm, "The world does not exist.");

#define SCRIPT_CHECK_RESULT(x) \
    if(x != USER_DATA_GET_SUCCESS) return sq_throwerror(vm, ScriptUtils::checkResultErrorMessage(x) );

#define SCRIPT_ASSERT_RESULT(XX) \
    UserDataGetResult __scriptError = XX; \
    assert(__scriptError == USER_DATA_GET_SUCCESS);

#define ASSERT_SQ_RESULT(xx) \
    bool __SQresult = SQ_SUCCEEDED(xx); \
    assert(__SQresult);

#define CHECK_PHYSICS() \
    if(SystemSettings::getPhysicsCompletelyDisabled()) return sq_throwerror(vm, "Physics is disabled.");

#define CHECK_DYNAMIC_PHYSICS() \
    if(SystemSettings::getDynamicPhysicsDisabled()) return sq_throwerror(vm, "Physics is disabled.");

#define ASSERT_DYNAMIC_PHYSICS() \
    assert(!SystemSettings::getDynamicPhysicsDisabled());

#define SCRIPT_RETURN_OGRE_ERROR(z, x) \
    std::string s(z); \
    s += x.getDescription(); \
    return sq_throwerror(vm, s.c_str()); \

namespace AV{
    //Specifies the result of functions which retrieve values from user data.
    enum UserDataGetResult{
        USER_DATA_GET_SUCCESS,
        USER_DATA_GET_TYPE_MISMATCH, //The type tags did not match up, i.e a user data was provided but it didn't have a matching tag.
        USER_DATA_GET_INCORRECT_TYPE, //Something other than a user data was found.
        USER_DATA_ERROR //A generic error
    };

    class ScriptUtils{
    public:

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

        static const char* checkResultErrorMessage(UserDataGetResult result){
            switch(result){
                case USER_DATA_GET_SUCCESS:
                    return "Success";
                case USER_DATA_GET_TYPE_MISMATCH:
                    return "Invalid object type";
                case USER_DATA_GET_INCORRECT_TYPE:
                    return "Incorrect type";
                default:
                    return "Error";
            }
        }

        static ChunkCoordinate getChunkCoordPopStack(HSQUIRRELVM vm){
            SQInteger slotX, slotY;

            sq_getinteger(vm, -1, &slotY);
            sq_getinteger(vm, -2, &slotX);
            const SQChar *mapName;
            sq_getstring(vm, -3, &mapName);

            sq_pop(vm, 3);

            return ChunkCoordinate((int)slotX, (int)slotY, Ogre::String(mapName));
        }

        static SlotPosition getSlotPositionPopStack(HSQUIRRELVM vm){
            SQInteger slotX, slotY;
            SQFloat x, y, z;
            sq_getfloat(vm, -1, &z);
            sq_getfloat(vm, -2, &y);
            sq_getfloat(vm, -3, &x);

            sq_getinteger(vm, -4, &slotY);
            sq_getinteger(vm, -5, &slotX);

            sq_pop(vm, 5);

            return SlotPosition(slotX, slotY, Ogre::Vector3(x, y, z));
        }

        template <size_t N>
        static void getFloatArray(HSQUIRRELVM vm, SQFloat array[N]){
            for(int i = 0; i < N; i++){
                sq_pushinteger(vm, i);

                sq_get(vm, -2);
                sq_getfloat(vm, -1, &(array[i]) );
                sq_pop(vm, 1);
            }
        }

        template <size_t N>
        static void getFloatValues(HSQUIRRELVM vm, SQInteger idx, SQFloat array[N]){
            for(int i = 0; i < N; i++){
                sq_getfloat(vm, idx + i, &(array[i]) );
            }
        }

        static void setupDelegateTable(HSQUIRRELVM vm, SQObject *obj, void(*setupFunc)(HSQUIRRELVM)){
            (*setupFunc)(vm);
            sq_resetobject(obj);
            sq_getstackobj(vm, -1, obj);
            sq_addref(vm, obj);
            sq_pop(vm, 1);
        }

        static void addFunction(HSQUIRRELVM v, SQFUNCTION f, const char *fname, int numParams = 0, const char *typeMask = ""){
            sq_pushstring(v, _SC(fname), -1);
            sq_newclosure(v,f,0);
            if(numParams != 0){
                sq_setparamscheck(v,numParams,_SC(typeMask));
            }
            sq_newslot(v,-3,SQFalse);
        }

        static void declareConstant(HSQUIRRELVM vm, const char* name, SQInteger val){
            sq_pushstring(vm, _SC(name), -1);
            sq_pushinteger(vm, val);
            sq_newslot(vm, -3 , false);
        }

        static void redirectFunctionMap(HSQUIRRELVM v, SQFUNCTION redirectFunction, const RedirectFunctionMap &rMap, bool redirect){
            auto it = rMap.begin();
            while(it != rMap.end()){
                if(redirect){
                    const TestFunctionEntry& entry = (*it).second;
                    addFunction(v, entry.function, (*it).first, entry.typeCount, entry.typeMask);
                }else{
                    addFunction(v, redirectFunction, (*it).first);
                }

                it++;
            }
        }

        static const char* typeToStr(SQObjectType type) {
            switch (type) {
                case OT_INTEGER: return "INTEGER";
                case OT_FLOAT: return "FLOAT";
                case OT_BOOL: return "BOOL";
                case OT_STRING: return "STRING";
                case OT_TABLE: return "TABLE";
                case OT_ARRAY: return "ARRAY";
                case OT_USERDATA: return "USERDATA";
                case OT_CLOSURE: return "CLOSURE";
                case OT_NATIVECLOSURE: return "NATIVECLOSURE";
                case OT_GENERATOR: return "GENERATOR";
                case OT_USERPOINTER: return "USERPOINTER";
                case OT_THREAD: return "THREAD";
                case OT_FUNCPROTO: return "FUNCPROTO";
                case OT_CLASS: return "CLASS";
                case OT_INSTANCE: return "INSTANCE";
                case OT_WEAKREF: return "WEAKREF";
                case OT_OUTER: return "OUTER";
                case OT_NULL: return "NULL";
                default: return "UNKNOWN";
            }
        }

        static void _debugStack(HSQUIRRELVM sq){
            int top = sq_gettop(sq);
            if(top <= 0){
              AV_WARN("Nothing in the stack!");
              return;
            }
            //This push root table sometimes causes problems.
            //sq_pushroottable(sq);
            while(top >= 0) {
                SQObjectType objectType = sq_gettype(sq, top);
                //Type type = Type(objectType);
                AV_INFO("stack index: {} type: {}", top, typeToStr(objectType));
                top--;
            }
        }

    };
}
