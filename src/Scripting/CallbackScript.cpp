#include "CallbackScript.h"

#include <sqstdio.h>

#include "Logger/Log.h"

#include "ScriptNamespace/ScriptUtils.h"

namespace AV{
    CallbackScript::CallbackScript(HSQUIRRELVM vm)
        : mVm(vm){
        sq_resetobject(&mMainClosure);
        sq_resetobject(&mMainTable);
    }

    bool CallbackScript::prepare(const Ogre::String& path){
        //TODO make this callable multiple times.
        if(!_compileMainClosure(path)) return false;
        if(!_createMainTable()) return false;
        if(!_callMainClosure()) return false;
        if(!_parseClosureTable()) return false;

        mPrepared = true;

        return true;
    }

    void CallbackScript::destroy(){
        //TODO implement this.
    }

    bool CallbackScript::call(const Ogre::String& functionName){
        if(!mPrepared) return false;

        HSQOBJECT closure = mClosureMap[functionName];
        sq_pushobject(mVm, closure);
        sq_pushobject(mVm, mMainTable);

        if(SQ_FAILED(sq_call(mVm, 1, false, false))){
            AV_ERROR("Call failed.");
            return false;
        }
        sq_pop(mVm, 1);

        return true;
    }

    bool CallbackScript::_compileMainClosure(const Ogre::String& path){
        if(SQ_FAILED(sqstd_loadfile(mVm, path.c_str(), true))){
            AV_ERROR("loading file failed");
            return false;
        }

        sq_resetobject(&mMainClosure);
        sq_getstackobj(mVm, -1, &mMainClosure);
        //Add a reference to it so it's not deleted on pop.
        sq_addref(mVm, &mMainClosure);
        sq_pop(mVm, 1);

        return true;
    }

    bool CallbackScript::_createMainTable(){
        sq_newtable(mVm);

        sq_resetobject(&mMainTable);
        sq_getstackobj(mVm, -1, &mMainTable);
        sq_addref(mVm, &mMainTable);
        //Remove the table from the stack
        sq_pop(mVm, 1);

        return true;
    }

    bool CallbackScript::_parseClosureTable(){
        sq_pushobject(mVm, mMainTable);

        sq_pushnull(mVm);  //null iterator
        while(SQ_SUCCEEDED(sq_next(mVm, -2))){
            SQObjectType objectType = sq_gettype(mVm, -1);

            if(objectType != OT_CLOSURE) {
                sq_pop(mVm, 2); //Pop the values if we're going to continue.
                continue;
            }

            const SQChar *key;
            sq_getstring(mVm, -2, &key);

            HSQOBJECT closure;

            // sq_release(mVm, &closure);
            // sq_resetobject(&closure);

            sq_getstackobj(mVm, -1, &closure);
            mClosureMap.insert(std::pair<Ogre::String, HSQOBJECT>(Ogre::String(key), closure));

            sq_pop(mVm, 2);
        }

        sq_pop(mVm, 2); //pop the null iterator and original table push

        return true;
    }

    bool CallbackScript::_callMainClosure(){
        /*
        At the moment I call the main closure once on load to setup all the other closures.
        A squirrel script is essentually just a big closure, which contains the other closures I actually want to call individually.
        By calling the main closure once with the table as the context, I'm able to insert the individual closures into the table.
        Then I can just iterate through the table and pull the values I want out.
        If I can find a way to not have to make this initial call that would be better, but I can't find one at the moment.
        */
        sq_pushobject(mVm, mMainClosure);
        sq_pushobject(mVm, mMainTable);

        if(SQ_FAILED(sq_call(mVm, 1, false, false))){
            AV_ERROR("Failed to call the main closure in the callback script {}");
            return false;
        }

        sq_pop(mVm, 1);
        return true;
    }

}
