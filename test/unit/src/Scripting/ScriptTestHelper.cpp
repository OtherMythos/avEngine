#include "ScriptTestHelper.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#define private public

#include "Scripting/ScriptManager.h"

HSQUIRRELVM ScriptTestHelper::mSqvm = sq_open(1024);

void ScriptTestHelper::initialise(){
    AV::ScriptManager::_setupVM(mSqvm);
}

bool ScriptTestHelper::executeStringInt(const std::string&s, int* i){
    int top = sq_gettop(mSqvm);

    _executeString(s);

    SQObjectType t = sq_gettype(mSqvm, -1);
    if(t == OT_NULL || t != OT_INTEGER){
        return false;
    }

    SQInteger val = 0;
    sq_getinteger(mSqvm, -1, &val);

    *i = val;

    sq_settop(mSqvm, top);
    return true;
}

bool ScriptTestHelper::executeStringBool(const std::string&s, bool* i){
    int top = sq_gettop(mSqvm);

    _executeString(s);

    SQObjectType t = sq_gettype(mSqvm, -1);
    if(t == OT_NULL || t != OT_BOOL){
        return false;
    }

    SQBool val = false;
    sq_getbool(mSqvm, -1, &val);

    *i = val;

    sq_settop(mSqvm, top);
    return true;
}

void ScriptTestHelper::executeString(const std::string&s){
    int top = sq_gettop(mSqvm);

    _executeString(s);

    sq_settop(mSqvm, top);
}

void ScriptTestHelper::_processSquirrelFailure(HSQUIRRELVM vm){
    const SQChar* sqErr;
    sq_getlasterror(vm);
    sq_tostring(vm, -1);
    sq_getstring(vm, -1, &sqErr);
    sq_pop(vm, 1);

    AV_ERROR(sqErr);
}

bool ScriptTestHelper::_executeString(const std::string& s){
    int top = sq_gettop(mSqvm);
    if(SQ_FAILED(sq_compilebuffer(mSqvm, s.c_str(), s.size(), "", false))){
        _processSquirrelFailure(mSqvm);
        sq_settop(mSqvm, top);
        return false;
    }

    sq_pushroottable(mSqvm);
    if(SQ_FAILED(sq_call(mSqvm, 1, 1, 1))){
        _processSquirrelFailure(mSqvm);
        sq_settop(mSqvm, top);
        return false;
    }

    return true;
}
