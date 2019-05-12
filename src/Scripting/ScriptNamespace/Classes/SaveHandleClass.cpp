#include "SaveHandleClass.h"

#include "Serialisation/SaveHandle.h"

namespace AV{
    SQMemberHandle SaveHandleClass::saveName;
    SQObject SaveHandleClass::classObject;
    
    void SaveHandleClass::setupClass(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("SaveHandle"), -1);
        sq_newclass(vm, 0);
        
        const char saveNameString[] = "saveName";
        
        sq_pushstring(vm, _SC(saveNameString), -1);
        sq_pushstring(vm, _SC(""), -1);
        sq_newslot(vm, -3, false);
        
        sq_pushstring(vm, _SC(saveNameString), -1);
        sq_getmemberhandle(vm, -2, &saveName);
        
        sq_resetobject(&classObject);
        sq_getstackobj(vm, -1, &classObject);
        
        sq_newslot(vm, -3, false);
    }
    
    void SaveHandleClass::saveHandleToInstance(HSQUIRRELVM vm, const SaveHandle& handle){
        sq_pushobject(vm, classObject);
        
        sq_createinstance(vm, -1);
        
        //The class itself isn't popped, which can get difficult.
        //So insted I get a reference to the new instance, pop it and the class, and then push the reference back.
        SQObject instanceObj;
        sq_getstackobj(vm,-1,&instanceObj);
        sq_addref(vm, &instanceObj);
        sq_pop(vm, 2);
        sq_pushobject(vm, instanceObj);
        
        sq_pushstring(vm, _SC(handle.saveName.c_str()), -1);
        sq_setbyhandle(vm, -2, &saveName);
    }
    
    SaveHandle SaveHandleClass::instanceToSaveHandle(HSQUIRRELVM vm){
        SaveHandle handle;
        const SQChar *saveNameString;
        
        sq_getbyhandle(vm, -1, &saveName);
        sq_getstring(vm, -1, &saveNameString);
        
        handle.saveName = saveNameString;
        
        return handle;
    }
}
