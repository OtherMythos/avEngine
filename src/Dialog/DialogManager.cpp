#include "DialogManager.h"

#include "Dialog/Compiler/DialogCompiler.h"
#include "Scripting/DialogScriptImplementation.h"
#include <cassert>
#include "Logger/Log.h"

namespace AV{
    DialogManager::DialogManager()
    : mImplementation(std::make_shared<DialogScriptImplementation>()),
      mCurrentDialog(EMPTY_DIALOG),
      mDialogSet(false){

    }

    DialogManager::~DialogManager(){

    }

    void DialogManager::initialise(){
        mImplementation->initialise();
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
        _handleTagEntry(t);
    }

    void DialogManager::_handleTagEntry(const TagEntry& t){
        switch(t.type){
            case TagType::TEXT_STRING:{
                mImplementation->notifyDialogString((*mCurrentDialog.stringList)[t.i]);
                _blockExecution();
                break;
            };
            case TagType::JMP:{
                _jumpToBlock(t.i);
                break;
            };
            default:{
                assert(false); //For the moment.
                break;
            }
        }
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
}
