#include "Vector3Class.h"

#include "Logger/Log.h"

namespace AV{
    SQMemberHandle Vector3Class::handleX;
    SQMemberHandle Vector3Class::handleY;
    SQMemberHandle Vector3Class::handleZ;

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
            default: return "UNKNOWN";
        }
    }


    void debugStack(HSQUIRRELVM sq){
        int top = sq_gettop(sq);
        if(top <= 0){
          std::cout << "Nothing in the stack!" << '\n';
          return;
        }
        //This push root table sometimes causes problems.
        //sq_pushroottable(sq);
        while(top >= 0) {
            SQObjectType objectType = sq_gettype(sq, top);
            //Type type = Type(objectType);
            std::cout << "stack index: " << top << " type: " << typeToStr(objectType) << std::endl;
            top--;
        }
    }

    SQInteger Vector3Class::vec3Constructor(HSQUIRRELVM vm){
        SQInteger nargs = sq_gettop(vm);
        //No arguments were passed in, or an invalid ammount, so nothing needs to be set.

        //If three values were passed in (plus the this), set them.
        if(nargs == 4){
            sq_setbyhandle(vm, -5, &handleZ);
            sq_setbyhandle(vm, -4, &handleY);
            sq_setbyhandle(vm, -3, &handleX);
        }

        return 0;
    }

    SQInteger Vector3Class::vec3AddMeta(HSQUIRRELVM vm){
        SQInteger nargs = sq_gettop(vm);

        //Get the values from the instances.
        SQFloat fx, fy, fz;
        sq_getbyhandle(vm, -2, &handleZ);
        sq_getfloat(vm, -1, &fz);
        sq_getbyhandle(vm, -3, &handleY);
        sq_getfloat(vm, -1, &fy);
        sq_getbyhandle(vm, -4, &handleX);
        sq_getfloat(vm, -1, &fx);
        sq_pop(vm, 3);

        SQFloat sx, sy, sz;
        sq_getbyhandle(vm, -1, &handleZ);
        sq_getfloat(vm, -1, &sz);
        sq_getbyhandle(vm, -2, &handleY);
        sq_getfloat(vm, -1, &sy);
        sq_getbyhandle(vm, -3, &handleX);
        sq_getfloat(vm, -1, &sx);
        sq_pop(vm, 3);

        //Get the class.
        sq_pushroottable(vm);
        sq_pushstring(vm, _SC("Vector3"), -1);
        sq_get(vm, -2);

        //Create the new instance.
        sq_createinstance(vm, -1);

        //Determine and push the floats.
        sq_pushfloat(vm, fx+sx);
        sq_pushfloat(vm, fy+sy);
        sq_pushfloat(vm, fz+sz);

        //Set them!
        sq_setbyhandle(vm, -4, &handleZ);
        sq_setbyhandle(vm, -3, &handleY);
        sq_setbyhandle(vm, -2, &handleX);

        return 1;
    }

    void Vector3Class::setupClass(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("Vector3"), -1);
        sq_newclass(vm, 0);

        sq_pushstring(vm, _SC("x"), -1);
        sq_pushfloat(vm, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("y"), -1);
        sq_pushfloat(vm, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("z"), -1);
        sq_pushfloat(vm, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("constructor"), -1);
        sq_newclosure(vm, vec3Constructor, 0);
        //Minus one for at least 1 parameter, this being the invisible 'this' variable. So the minimum is 0 parameters.
        //Param check itself seems quite limited in what you're able to specify.
        //I can't specify no arguments or exactly three.
        //Seemingly the best I can do is specify at least one argument, and three numbers.
        //This will allow things like (10, 20, 30), but will also allow things like (10, 20, 30, 40).
        //Much of the error checking will have to be done in the c++.
        sq_setparamscheck(vm,-1,_SC("xnnn"));
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("_add"), -1);
        sq_newclosure(vm, vec3AddMeta, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("x"), -1);
        sq_getmemberhandle(vm, -2, &handleX);

        sq_pushstring(vm, _SC("y"), -1);
        sq_getmemberhandle(vm, -2, &handleY);

        sq_pushstring(vm, _SC("z"), -1);
        sq_getmemberhandle(vm, -2, &handleZ);


        sq_newslot(vm, -3, false);
    }
}
