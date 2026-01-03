#include "DialogManager.h"

#include "System/BaseSingleton.h"
#include "Scripting/ScriptVM.h"
#include "Scripting/ScriptManager.h"
#include "Scripting/Script/CallbackScript.h"

#include "Dialog/Compiler/DialogCompiler.h"
#include "Scripting/DialogScriptImplementation.h"
#include <cassert>
#include "Logger/Log.h"

namespace AV{
    DialogManager::DialogManager()
    //: mImplementation(),
      :mCurrentDialog(EMPTY_DIALOG),
      mDialogSet(false){

    }

    DialogManager::~DialogManager(){

    }

    void DialogManager::shutdown(){
        unsetCompiledDialog();
        mImplementation.reset();
        mDialogScripts.clear();
    }

    void DialogManager::initialise(){
        mImplementation = std::make_shared<DialogScriptImplementation>();
        mImplementation->initialise();

        mLocalRegistry = std::make_shared<ValueRegistry>();
    }

    void DialogManager::setCompiledDialog(const CompiledDialog& d){
        if(d.empty()) return;

        mCurrentDialog = d;
        mDialogSet = true;
    }

    void DialogManager::unsetCompiledDialog(){
        if(mDestroyCompiledDialogOnEnd){
            mCurrentDialog.destroy();
        }
        mDestroyCompiledDialogOnEnd = false;

        mCurrentDialog = EMPTY_DIALOG;
        mDialogSet = false;
    }

    bool DialogManager::compileAndRunDialog(const std::string& path){
        CompiledDialog d;
        DialogCompiler compiler;
        if(!compiler.compileScript(path, d)) return false;

        mDestroyCompiledDialogOnEnd = true;

        beginExecution(d);

        return true;
    }

    bool DialogManager::beginExecution(const CompiledDialog& d, BlockId startBlock){
        if(!d.empty()){
            setCompiledDialog(d);
        }
        if(!mDialogSet) return false; //Nothing is set so there's nothing to execute.
        if(!mImplementation->isSetupCorrectly()){
            AV_ERROR("No dialog script implementation was provided. No dialog scripts are able to execute.");
            unsetCompiledDialog();
            return false;
        }
        if((*mCurrentDialog.blockMap).find(startBlock) == (*mCurrentDialog.blockMap).end()){
            //An invalid block was provided.
            AV_ERROR("No dialog block with id {} was found.", startBlock);
            unsetCompiledDialog();
            return false;
        }

        mExecuting = true;
        mBlocked = false;
        mExecutingBlock = startBlock;
        mExecTagIndex = 0;

        _checkDialogPrerequisites();

        mImplementation->notifyDialogExecutionBegin();

        return true;
    }

    bool DialogManager::notifyDialogDeletion(CompiledDialog* dialog){
        //The assumption is that if this is true the dialog manager is managing deletion, so don't bother doing anything.
        if(mDestroyCompiledDialogOnEnd) return false;
        if(mCurrentDialog != *dialog) return false;

        _endExecution();

        return true;
    }

    void DialogManager::update(){
        if(!mDialogSet || !mExecuting) return;

        if(mSleeping){
            //Check if the time to sleep has finished.
            if(!_checkSleepInterval()) return;
        }

        while(!mBlocked && mExecuting){
            _executeDialog();
        }
    }

    void DialogManager::unblock(){
        mBlocked = false;
    }

    bool DialogManager::_checkDialogPrerequisites(){
        mDialogScripts.clear();

        for(const TagEntry& e : *(mCurrentDialog.headerInformation) ){
            bool containsVariable = _tagContainsVariable(e.type);
            TagType tt = _stripVariableFlag(e.type);

            switch(tt){
                case TagType::SCRIPT:{
                    //This is a script declaration, meaning that the script needs to be created.
                    std::string targetPath;
                    int targetId = 0;
                    if(containsVariable){
                        const vEntry2& vEntry = (*mCurrentDialog.vEntry2List)[e.i];
                        bool ret = true;
                        _readStringVariable(targetPath, vEntry.x, ret, tt, "path");
                        _readIntVariable(targetId, vEntry.y, ret, tt, "id");
                        if(!ret) return false;
                    }else{
                        const Entry2& entry = (*mCurrentDialog.entry2List)[e.i];
                        targetPath = (*mCurrentDialog.stringList)[entry.x];
                        targetId = entry.y;
                    }

                    std::shared_ptr<CallbackScript> s = BaseSingleton::getScriptManager()->loadScript(targetPath);
                    assert(mDialogScripts.find(targetId) == mDialogScripts.end());
                    mDialogScripts[targetId] = s;
                }
                default: break;
            }
        }

        return true;
    }

    void DialogManager::_endExecution(){
        mExecuting = false;
        mBlocked = false;

        mImplementation->notifyDialogExecutionEnded();
        unsetCompiledDialog();
        mLocalRegistry->clear();

        mDialogScripts.clear();
    }

    void DialogManager::_executeDialog(){
        auto b = (*mCurrentDialog.blockMap).find(mExecutingBlock);
        assert(b != (*mCurrentDialog.blockMap).end()); //By this point we should be sure the block exists.

        const BlockContentList& list = *((*b).second);

        if(mExecTagIndex >= list.size()){
            //The current tag index is greater than entries in this block, meaning there are none left. In this case the execution ends.
            _endExecution();
            return;
        }

        assert(mExecTagIndex < list.size());

        const TagEntry& t = list[mExecTagIndex++];
        bool runtimeResult = _handleTagEntry(t);
        if(!runtimeResult){
            //There was a runtime error during the execution of the dialog.
            _printErrorMessage();
            _endExecution();
            return;
        }
    }

    bool DialogManager::_handleTagEntry(const TagEntry& t){
        bool containsVariable = _tagContainsVariable(t.type);
        TagType tt = _stripVariableFlag(t.type);

        switch(tt){
            case TagType::TEXT_STRING:{
                if(containsVariable){
                    std::string retString = _produceDialogVariableString((*mCurrentDialog.stringList)[t.i], "var");
                    mImplementation->notifyDialogString(retString);
                }else mImplementation->notifyDialogString((*mCurrentDialog.stringList)[t.i]);
                _blockExecution();
                mRequestedDialogClose = false;
                break;
            };
            case TagType::JMP:{
                int jmpIndex = 0;
                if(containsVariable){
                    const VariableAttribute& att = (*mCurrentDialog.vEntry1List)[t.i];
                    bool ret = true;
                    _readIntVariable(jmpIndex, att, ret, tt, "id");
                    if(!ret) return false;
                }else{
                    jmpIndex = t.i;
                }
                return _jumpToBlock(jmpIndex);
                break;
            };
            case TagType::SLEEP:{
                int sleepVal = 0;
                if(containsVariable){
                    const VariableAttribute& att = (*mCurrentDialog.vEntry1List)[t.i];
                    bool ret = true;
                    _readIntVariable(sleepVal, att, ret, tt, "l");
                    if(!ret) return false;
                }else{
                    sleepVal = t.i;
                }
                _beginSleep(sleepVal);
                _blockExecution();
                break;
            };
            case TagType::ACTOR_MOVE_TO:{
                Entry4 outEntry;
                if(containsVariable){
                    const vEntry4& e = (*mCurrentDialog.vEntry4List)[t.i];
                    bool ret = true;
                    _readIntVariable(outEntry.w, e.w, ret, tt, "a");
                    _readIntVariable(outEntry.x, e.x, ret, tt, "x");
                    _readIntVariable(outEntry.y, e.y, ret, tt, "y");
                    _readIntVariable(outEntry.z, e.z, ret, tt, "z");
                    if(!ret) return false;
                }else{
                    outEntry = (*mCurrentDialog.entry4List)[t.i];
                }
                mImplementation->notifyActorMoveTo(outEntry);
                _blockExecution();
                _notifyHideDialog();
                break;
            };
            case TagType::ACTOR_CHANGE_DIRECTION:{
                Entry2 outEntry;
                if(containsVariable){
                    const vEntry2& e = (*mCurrentDialog.vEntry2List)[t.i];
                    bool ret = true;
                    _readIntVariable(outEntry.x, e.x, ret, tt, "a");
                    _readIntVariable(outEntry.y, e.y, ret, tt, "d");
                    if(!ret) return false;
                }else{
                    outEntry = (*mCurrentDialog.entry2List)[t.i];
                }
                mImplementation->notifyActorChangeDirection(outEntry);
                //TODO I might want to make this blocking, maybe if there has to be an animation during the change.
                break;
            };
            case TagType::HIDE_DIALOG_WINDOW:{
                _notifyHideDialog();
                break;
            };
            case TagType::SCRIPT:{
                int scriptId = 0;
                std::string funcName;
                int blockId = -1;
                int varId = -1;
                int totalVariables = -1;
                if(containsVariable){
                    const vEntry4& e = (*mCurrentDialog.vEntry4List)[t.i];

                    bool ret = true;
                    _readIntVariable(scriptId, e.x, ret, tt, "id");
                    _readStringVariable(funcName, e.y, ret, tt, "func");

                    varId = e.z.i;
                    totalVariables = (e.w.i) & 0x7;

                    if(e.w.i & (1 << 5)){
                        const vEntry4& ee = (*mCurrentDialog.vEntry4List)[t.i+1];
                        bool blockRet = true;
                        _readIntVariable(blockId, ee.x, blockRet, tt, "id");
                    }

                    if(!ret) return false;
                }else{
                    const Entry4& e = (*mCurrentDialog.entry4List)[t.i];
                    scriptId = e.x;
                    funcName = (*mCurrentDialog.stringList)[e.y];

                    if(e.w & (1 << 5)){
                        const Entry4& ee = (*mCurrentDialog.entry4List)[t.i+1];
                        blockId = ee.x;
                    }

                    varId = e.z;
                    totalVariables = (e.w) & 0x7;
                }
                bool jumpToBlock = false;
                if(!_executeScriptTag(scriptId, funcName, varId, totalVariables, blockId, &jumpToBlock)) return false;
                if(jumpToBlock){
                    _jumpToBlock(blockId);
                }
                break;
            }
            case TagType::SET:{
                const vEntry2& e = (*mCurrentDialog.vEntry2List)[t.i];

                //The first entry is guaranteed to be a variable hash each time.
                IdString key;
                key.mHash = e.x.mVarHash;

                VariableCharContents c;
                _readVariableChar(e.y._varData, c);
                if(c.isVariable){
                    const void* v;
                    RegistryType t;
                    RegistryLookup result = _getRegistry(c.isGlobal)->getValue(e.y.mVarHash, v, t);
                    if(!lookupSuccess(result)){
                        mErrorReason = {"Error reading registry value."};
                        //TODO improve this error message.
                        //And also add some sort of function to unify the retreival procedure.
                        return false;
                    }
                    switch(t){
                        case RegistryType::STRING:{
                            const std::string* s = static_cast<const std::string*>(v);
                            _getRegistry(true)->setStringValue(key, *s);
                            break;
                        }
                        case RegistryType::FLOAT:{
                            const float* f = static_cast<const float*>(v);
                            _getRegistry(true)->setFloatValue(key, *f);
                            break;
                        }
                        case RegistryType::INT:{
                            const int* i = static_cast<const int*>(v);
                            _getRegistry(true)->setIntValue(key, *i);
                            break;
                        }
                        case RegistryType::BOOLEAN:{
                            const bool* b = static_cast<const bool*>(v);
                            _getRegistry(true)->setBoolValue(key, *b);
                            break;
                        }
                        default:{
                            assert(false);
                        }
                    }

                }else{
                    switch(c.type){
                        case AttributeType::INT: _getRegistry(true)->setIntValue(key, e.y.i); break;
                        case AttributeType::FLOAT: _getRegistry(true)->setFloatValue(key, e.y.f); break;
                        case AttributeType::BOOLEAN: _getRegistry(true)->setBoolValue(key, e.y.b); break;
                        case AttributeType::STRING:{
                            const std::string& s = (*mCurrentDialog.stringList)[e.y.i];
                            _getRegistry(true)->setStringValue(key, s);
                            break;
                        }
                        default: assert(false);
                    }
                }
                break;
            }
            case TagType::OPTION:{
                const vEntry4& baseEntry = (*mCurrentDialog.vEntry4List)[t.i];

                const VariableAttribute* targetBaseEntries[4] = {
                    &(baseEntry.x),
                    &(baseEntry.y),
                    &(baseEntry.z),
                    &(baseEntry.w)
                };
                const std::string* targetOptions[4];
                memset(&targetOptions, 0, sizeof(targetOptions));
                memset(&mDialogOptionTargets, -1, sizeof(mDialogOptionTargets));
                for(uint8 i = 0; i < 4; i++){
                    const VariableAttribute* targetAttrib = targetBaseEntries[i];
                    //Check if the option is populated.
                    if(targetAttrib->_varData == 0) continue;

                    int targetVariable = targetAttrib->i;

                    const vEntry2& varEntry = (*mCurrentDialog.vEntry2List)[targetVariable];
                    assert(varEntry.x.i < mCurrentDialog.stringList->size());
                    const std::string& targetString = (*mCurrentDialog.stringList)[varEntry.x.i];
                    targetOptions[i] = &targetString;

                    //Read the target id from the list.
                    bool ret = true;
                    _readIntVariable(mDialogOptionTargets[i], varEntry.y, ret, tt, "id");
                    if(!ret) return false;
                }

                mOptionActive = true;
                mImplementation->notifyOption(&targetOptions[0]);
                _blockExecution();

                break;
            }
            case TagType::SWITCH:{
                const vEntry4& blockList = (*mCurrentDialog.vEntry4List)[t.i];
                const vEntry4& testList = (*mCurrentDialog.vEntry4List)[t.i+1];

                for(uint8 i = 0; i < 4; i++){
                    const VariableAttribute& targetBlock = blockList[i];
                    if((targetBlock._varData & (1 << 7)) <= 0) continue;

                    const VariableAttribute& targetTest = testList[i];
                    bool ret = true;
                    bool testValue = false;
                    _readBoolVariable(testValue, targetTest, ret, tt, "test");
                    if(!ret) return false;
                    if(!testValue) continue;

                    //In this case the test passed, so jump to the target block.
                    ret = true;
                    int jmpIndex = 0;
                    _readIntVariable(jmpIndex, targetBlock, ret, tt, "id");
                    if(!ret) return false;

                    return _jumpToBlock(jmpIndex);
                }

                break;
            }
            default:{
                assert(false); //For the moment.
                break;
            }
        }

        return true;
    }

    DialogManager::DialogSpecifyOptionErrors DialogManager::specifyOption(int option){
        if(option < 0 || option >= MAX_DIALOG_OPTIONS) return DialogSpecifyOptionErrors::INVALID_ID;
        if(!mOptionActive) return DialogSpecifyOptionErrors::OPTION_NOT_ACTIVE;

        int target = mDialogOptionTargets[option];

        mOptionActive = false;
        bool result = _jumpToBlock(target);
        if(!result){
            return DialogSpecifyOptionErrors::INVALID_TARGET;
        }
        unblock();

        return DialogSpecifyOptionErrors::SUCCESS;
    }

    bool DialogManager::_checkSleepInterval(){
        auto t = std::chrono::steady_clock::now() - sleepBeginTime;
        auto tt = std::chrono::duration_cast<std::chrono::milliseconds>(t);
        if(tt.count() >= mSleepInterval){
            mSleepInterval = -1;
            mSleeping = false;
            unblock();
            return true;
        }

        return false;
    }

    StringListType* _stringsList = 0;
    ValueRegistry* _globalRegistry = 0;
    ValueRegistry* _localRegistry = 0;
    void pushSquirrelValForVariable(HSQUIRRELVM vm, const VariableAttribute* at){
        DialogManager::VariableCharContents c;
        DialogManager::_readVariableChar(at->_varData, c);

        if(c.isVariable){
            ValueRegistry* targetRegistry = c.isGlobal ? _globalRegistry : _localRegistry;
            assert(targetRegistry);

            const void* v;
            RegistryType t;
            RegistryLookup result;

            IdString val;
            val.mHash = at->mVarHash;
            result = targetRegistry->getValue(val, v, t);
            if(!lookupSuccess(result)){
                //TODO there should be a bit more error description here.
                //Maybe not canceling execution, but something.
                sq_pushnull(vm);
                return;
            }
            switch(t){
                case RegistryType::STRING:{
                    const std::string* s = static_cast<const std::string*>(v);
                    sq_pushstring(vm, s->c_str(), -1);
                    break;
                }
                case RegistryType::FLOAT:{
                    const float* f = static_cast<const float*>(v);
                    sq_pushfloat(vm, *f);
                    break;
                }
                case RegistryType::BOOLEAN:{
                    const bool* b = static_cast<const bool*>(v);
                    sq_pushbool(vm, *b);
                    break;
                }
                case RegistryType::INT:{
                    const int* i = static_cast<const int*>(v);
                    sq_pushinteger(vm, *i);
                    break;
                }
                default:{
                    assert(false);
                }
            }

        }else{
            switch(c.type){
                case AttributeType::INT: sq_pushinteger(vm, at->i); break;
                case AttributeType::FLOAT: sq_pushfloat(vm, at->f); break;
                case AttributeType::BOOLEAN: sq_pushbool(vm, at->b); break;
                case AttributeType::STRING:{
                    const std::string& s = (*_stringsList)[at->i];
                    sq_pushstring(vm, s.c_str(), -1);
                    break;
                }
                default: break;
            }
        }
    }

    int _totalVariables;
    const vEntry4* _targetEntry = 0;
    SQInteger populateScriptTag(HSQUIRRELVM vm){
        assert(_totalVariables > 0); //If this is getting called there should be some variables.
        assert(_targetEntry);

        const vEntry4& vals = *_targetEntry;
        const VariableAttribute* varAtt[4] = {&vals.x, &vals.y, &vals.z, &vals.w};
        for(int i = 0; i < _totalVariables; i++){
            pushSquirrelValForVariable(vm, varAtt[i]);
        }

        int targetVariables = _totalVariables;

        _totalVariables = -1;
        _targetEntry = 0;
        _stringsList = 0;
        _localRegistry = 0;
        _globalRegistry = 0;

        return targetVariables + 1;
    }

    bool _scriptTagReturn = false;
    SQInteger returnFunctionScriptTag(HSQUIRRELVM vm){
        if(sq_gettype(vm, -1) != OT_BOOL) return 0;
        SQBool scriptBool;
        sq_getbool(vm, -1, &scriptBool);
        _scriptTagReturn = scriptBool;

        sq_poptop(vm);

        return 0;
    }

    bool DialogManager::_executeScriptTag(int scriptIdx, const std::string& funcName, int variablesId, int totalVariables, int jumpBlockId, bool* returnValue){
        _scriptTagReturn = false;

        auto it = mDialogScripts.find(scriptIdx);
        if(it == mDialogScripts.end()){
            mErrorReason = {"No script with the id " + std::to_string(scriptIdx) + " could be found."};
            return false;
        }
        std::shared_ptr<CallbackScript> s = (*it).second;

        PopulateFunction func = 0;
        if(totalVariables > 0){
            //This script call contained variables, so get those.
            assert(variablesId >= 0);
            _targetEntry = &((*mCurrentDialog.vEntry4List)[variablesId]);
            _totalVariables = totalVariables;
            _stringsList = mCurrentDialog.stringList;
            _localRegistry = mLocalRegistry.get();
            _globalRegistry = BaseSingleton::getGlobalRegistry().get();
            func = populateScriptTag;
        }

        ReturnFunction retFunc = 0;
        if(jumpBlockId >= 0){
            retFunc = returnFunctionScriptTag;
        }

        if(!s->call(funcName, func, retFunc)){
            mErrorReason = {"Error calling script of path " + s->getFilePath() + " with function " + funcName};
            return false;
        }

        *returnValue = _scriptTagReturn;

        return true;
    }

    void DialogManager::_blockExecution(){
        mBlocked = true;
    }

    bool DialogManager::_jumpToBlock(BlockId target){
        if(target < 0){
            _endExecution();
            return true;
        }

        const BlockMapType& b = (*mCurrentDialog.blockMap);
        if(b.find(target) == b.end()){
            mErrorReason = {
                "Could not jump to block with id " + std::to_string(target) + " as it was not found."
            };
            return false;
        }

        mExecutingBlock = target;
        mExecTagIndex = 0;
        return true;
    }

    void DialogManager::_beginSleep(int milliseconds){
        sleepBeginTime = std::chrono::steady_clock::now();
        mSleeping = true;
        mSleepInterval = milliseconds;
    }

    void DialogManager::_notifyHideDialog(){
        if(mRequestedDialogClose) return; //Nothing to do.

        mImplementation->notifyShouldCloseDialog();

        mRequestedDialogClose = true;
    }

    std::string DialogManager::_produceDialogVariableString(const std::string& initString, const std::string& replaceString){
        std::string retString = initString;

        bool found = false;
        std::string::const_iterator f;
        std::string::const_iterator it = retString.begin();
        while(it != retString.end()){
            if(*it == '$' || *it == '#'){
                if(!found){
                    f = it;
                    found = true;
                }else{
                    const std::string s = _determineStringVariable(retString, f + 1, it, *it == '$');
                    retString.replace(f, it + 1, s);
                    //it = f;
                    //it = f worked fine on linux but causes crashes on mac. Maybe have a look at this at some point.
                    it = retString.begin();
                    found = false;
                }
            }
            it++;
        }

        return retString;
    }

    std::string DialogManager::_determineStringVariable(const std::string& str, std::string::const_iterator f, std::string::const_iterator s, bool globalVariable){
        IdString val(std::string(f, s));

        const void* v;
        RegistryType t;
        RegistryLookup result;
        if(globalVariable){
            result = BaseSingleton::getGlobalRegistry()->getValue(val, v, t);
        }else{
            result = mLocalRegistry->getValue(val, v, t);
        }

        if(!lookupSuccess(result)){
            //There was some sort of error printing finding that value.
            return "<Error>";
        }

        switch(t){
            case RegistryType::STRING:{
                const std::string* s = static_cast<const std::string*>(v);
                return *s;
            }
            case RegistryType::FLOAT:{
                const float* f = static_cast<const float*>(v);
                return std::to_string(*f);
            }
            case RegistryType::INT:{
                const int* i = static_cast<const int*>(v);
                return std::to_string(*i);
                break;
            }
            case RegistryType::BOOLEAN:{
                const bool* b = static_cast<const bool*>(v);
                return *b ? "true" : "false";
                break;
            }
            default:{
                assert(false);
            }
        }

        return "<Error>";
    }

    void DialogManager::_readVariableChar(char c, VariableCharContents& out){
        out.isVariable = static_cast<bool>(c & 0x1);
        out.isGlobal = static_cast<bool>(c & 0x2);

        out.type = static_cast<AttributeType>(c >> 2u);
    }

    std::shared_ptr<ValueRegistry> DialogManager::_getRegistry(bool registry){
        return registry ? BaseSingleton::getGlobalRegistry() : mLocalRegistry;
    }

    void DialogManager::_printErrorMessage(){
        AV_ERROR("The dialog system encountered a runtime error");
        AV_ERROR("  Block {} tag {}", mExecutingBlock, mExecTagIndex);
        for(const std::string& s : mErrorReason){
            AV_ERROR(s);
        }
        mErrorReason.clear();
    }

    template <class T>
    void DialogManager::_readVariable(RegistryLookup(ValueRegistry::*funcPtr)(IdString, T&), T& out, const VariableAttribute& e, bool& outVal, TagType t, const char* attribName, int* stringId, bool* isConstant){
        if(!outVal) return;
        VariableCharContents ax;
        _readVariableChar(e._varData, ax);
        if(ax.isVariable){
            IdString id;
            id.mHash = e.mVarHash;
            auto reg = _getRegistry(ax.isGlobal);
            RegistryLookup result = ((*(reg.get())).*funcPtr)(id, out);
            if(!lookupSuccess(result)){
                outVal = false;
                const char* c = ax.isGlobal ? "global" : "local";
                if(result == REGISTRY_MISSING) {
                    mErrorReason = {
                        "Registry missing value " + id.getReleaseText() + " in the " + c + " registry.",
                        "TagType: " + std::string(tagTypeString(t)),
                        "When processing attribute: " + std::string(attribName)
                    };
                }
                else if(result == REGISTRY_MISMATCH){
                    mErrorReason = {
                        "Registry type mismatch at entry " + id.getReleaseText() + " in the " + c + " registry.",
                        "Expected: " + std::string(attributeTypeString(ax.type)),
                        "Instead received a type of: " + std::string(_getRegistry(ax.isGlobal)->getStringTypeOfEntry(id)),
                        "TagType: " + std::string(tagTypeString(t)),
                        "When processing attribute: " + std::string(attribName)
                    };
                }
                return;
            }
            #ifdef DEBUGGING_TOOLS
                //If debugging tools are enabled print out every single variable read.
                const char* c = ax.isGlobal ? "global" : "local";
                AV_INFO("Read variable of type {} from the {} registry.", attributeTypeString(ax.type), c);
                AV_INFO("\tReturned value of {}", out);
            #endif
        }else {
            switch(ax.type){
                case AttributeType::STRING:{
                    //The string requires special steps as the string value is stored by id.
                    //This function really just returns an id to the strings list.
                    assert(stringId);
                    assert(isConstant);
                    *stringId = e.i;
                    *isConstant = true;
                    break;
                }
                case AttributeType::INT: out = e.i; break;
                case AttributeType::BOOLEAN: out = e.b; break;
                case AttributeType::FLOAT: out = e.f; break;
                default: out = e.i; break;
            }
        }
    }

    void DialogManager::_readIntVariable(int& out, const VariableAttribute& e, bool& outVal, TagType t, const char* attribName){
        _readVariable<int>(&ValueRegistry::getIntValue, out, e, outVal, t, attribName);
    }

    void DialogManager::_readBoolVariable(bool& out, const VariableAttribute& e, bool& outVal, TagType t, const char* attribName){
        _readVariable<bool>(&ValueRegistry::getBoolValue, out, e, outVal, t, attribName);
    }

    void DialogManager::_readStringVariable(std::string& out, const VariableAttribute& e, bool& outVal, TagType t, const char* attribName){
        int targetId = -1;
        bool isConstant = false;
        _readVariable<std::string>(&ValueRegistry::getStringValue, out, e, outVal, t, attribName, &targetId, &isConstant);

        //If it was a constant value, the function returns an id to the strings list rather than populating
        if(isConstant){
            assert(targetId >= 0);
            out = (*mCurrentDialog.stringList)[targetId];
        }
    }
}
