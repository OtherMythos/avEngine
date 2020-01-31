#include "DialogManager.h"

#include "System/BaseSingleton.h"
#include "System/Registry/ValueRegistry.h"
#include "OgreIdString.h"

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

    void DialogManager::beginExecution(const CompiledDialog& d, BlockId startBlock){
        if(!d.empty()){
            setCompiledDialog(d);
        }
        if(!mDialogSet) return; //Nothing is set so there's nothing to execute.
        if(!mImplementation->isSetupCorrectly()){
            AV_ERROR("No dialog script implementation was provided. No dialog scripts are able to execute.");
            unsetCompiledDialog();
            return;
        }

        mExecuting = true;
        mBlocked = false;
        mExecutingBlock = startBlock;
        mExecTagIndex = 0;

        mImplementation->notifyDialogExecutionBegin();
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

    void DialogManager::_endExecution(){
        mExecuting = false;
        mBlocked = false;

        mImplementation->notifyDialogExecutionEnded();
        unsetCompiledDialog();
        mLocalRegistry->clear();
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
                mRequestedDiaogClose = false;
                break;
            };
            case TagType::JMP:{
                int jmpIndex = 0;
                if(containsVariable){
                    const VariableAttribute& att = (*mCurrentDialog.vEntry1List)[t.i];
                    bool ret = true;
                    _readVariable(jmpIndex, att, ret, tt, "id");
                    if(!ret) return false;
                }else{
                    jmpIndex = t.i;
                }
                _jumpToBlock(jmpIndex);
                break;
            };
            case TagType::SLEEP:{
                int sleepVal = 0;
                if(containsVariable){
                    const VariableAttribute& att = (*mCurrentDialog.vEntry1List)[t.i];
                    bool ret = true;
                    _readVariable(sleepVal, att, ret, tt, "l");
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
                    _readVariable(outEntry.w, e.w, ret, tt, "a");
                    _readVariable(outEntry.x, e.x, ret, tt, "x");
                    _readVariable(outEntry.y, e.y, ret, tt, "y");
                    _readVariable(outEntry.z, e.z, ret, tt, "z");
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
                    _readVariable(outEntry.x, e.x, ret, tt, "a");
                    _readVariable(outEntry.y, e.y, ret, tt, "d");
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
            default:{
                assert(false); //For the moment.
                break;
            }
        }

        return true;
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

    void DialogManager::_blockExecution(){
        mBlocked = true;
    }

    void DialogManager::_jumpToBlock(BlockId target){
        const BlockMapType& b = (*mCurrentDialog.blockMap);
        assert(b.find(target) != b.end());

        mExecutingBlock = target;
        mExecTagIndex = 0;
    }

    void DialogManager::_beginSleep(int milliseconds){
        sleepBeginTime = std::chrono::steady_clock::now();
        mSleeping = true;
        mSleepInterval = milliseconds;
    }

    void DialogManager::_notifyHideDialog(){
        if(mRequestedDiaogClose) return; //Nothing to do.

        mImplementation->notifyShouldCloseDialog();

        mRequestedDiaogClose = true;
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
        Ogre::IdString val(std::string(f, s));

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

    void DialogManager::_readVariable(int& out, const VariableAttribute& e, bool& outVal, TagType t, const char* attribName){
        if(!outVal) return;
        VariableCharContents ax;
        _readVariableChar(e._varData, ax);
        if(ax.isVariable){
            Ogre::IdString id;
            id.mHash = e.mVarHash;
            RegistryLookup result = _getRegistry(ax.isGlobal)->getIntValue(id, out);
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
        }else out = e.i;
    }
}
