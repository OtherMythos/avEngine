#include "CallbackScript.h"

#include <sqstdio.h>

#include "Logger/Log.h"
#include "System/Util/PathUtils.h"


namespace AV{
    CallbackScript::CallbackScript(HSQUIRRELVM vm)
        : mVm(vm),
          mInitialised(true){
        sq_resetobject(&mMainClosure);
        sq_resetobject(&mMainTable);
    }

    CallbackScript::CallbackScript(){
        sq_resetobject(&mMainClosure);
        sq_resetobject(&mMainTable);
    }

    CallbackScript::~CallbackScript(){
        release();
    }

    void CallbackScript::initialise(HSQUIRRELVM vm){
        mVm = vm;
        mInitialised = true;
    }

    bool CallbackScript::prepare(const Ogre::String& path){
        if(!mInitialised) {
            AV_ERROR("Please initialise your CallbackScript with a VM before preparing it.");
            return false;
        }

        if(mPrepared){
            release();
        }

        filePath = path;

        std::string outString;
        formatResToPath(path, outString);

        if(!_compileMainClosure(outString)) return false;
        if(!_createMainTable()) return false;
        if(!_callMainClosure()) return false;
        if(!_parseClosureTable()) return false;

        mPrepared = true;

        return true;
    }

    void CallbackScript::release(){
        if(!mInitialised) return;
        //Doesn't matter if the script has not been prepared.

        //Theoretically this should also release the closures inside the table.
        //TODO confirm this.
        sq_release(mVm, &mMainTable);
        sq_release(mVm, &mMainClosure);

        mClosureMap.clear();
        mClosures.clear();
        filePath = "";
        mPrepared = false;
    }

    int CallbackScript::getCallbackId(const Ogre::String& functionName){
        auto it = mClosureMap.find(functionName);

        if(it == mClosureMap.end()) return -1;

        return (*it).second;
    }

    bool CallbackScript::_call(int closureId, PopulateFunction func){
        if(!mInitialised) return false;
        if(!mPrepared) return false;

        HSQOBJECT closure = mClosures[closureId];

        sq_pushobject(mVm, closure);
        sq_pushobject(mVm, mMainTable);

        SQInteger paramCount = 1;
        if(func){
            paramCount = (func)(mVm);
        }

        if(SQ_FAILED(sq_call(mVm, paramCount, false, true))){
            return false;
        }
        sq_pop(mVm, 1);

        return true;
    }

    bool CallbackScript::call(int closureId, PopulateFunction func){
        if(closureId < 0 || closureId >= mClosures.size()) return false;

        return _call(closureId, func);
    }

    bool CallbackScript::call(const Ogre::String& functionName, PopulateFunction func){
        auto it = mClosureMap.find(functionName);
        if (it == mClosureMap.end()){
            return false;
        }

        return _call((*it).second, func);
    }

    bool CallbackScript::_compileMainClosure(const Ogre::String& path){
        if(SQ_FAILED(sqstd_loadfile(mVm, path.c_str(), SQTrue))){
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
            mClosures.push_back(closure);
            //-1 because arrays start at 0
            mClosureMap.insert({Ogre::IdString(key), mClosures.size() - 1});

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

        if(SQ_FAILED(sq_call(mVm, 1, false, true))){
            //AV_ERROR("Failed to call the main closure in the callback script {}", filePath);
            return false;
        }

        sq_pop(mVm, 1);
        return true;
    }

}
