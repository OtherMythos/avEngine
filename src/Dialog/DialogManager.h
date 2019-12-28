#pragma once

#include "Compiler/DialogScriptData.h"

namespace AV{
    class DialogScriptImplementation;

    class DialogManager{
    public:
        DialogManager();
        ~DialogManager();

        /**
        Begin the execution of a compiled dialog script.
        */
        void beginExecution(const CompiledDialog& d = EMPTY_DIALOG, BlockId startBlock = 0);
        void setCompiledDialog(const CompiledDialog& d);
        void unsetCompiledDialog();

        //Unblock the dialog execution.
        void unblock();

        void update();

        void initialise();

    private:
        std::shared_ptr<DialogScriptImplementation> mImplementation;
        CompiledDialog mCurrentDialog;

        //Whether a compiled dialog script has actually been set.
        bool mDialogSet;
        //Whether a dialog script is being run at the moment.
        bool mExecuting;
        //Whether the current execution of the script has been blocked.
        bool mBlocked;

        BlockId mExecutingBlock = 0;
        int mExecTagIndex = 0;

        void _executeDialog();
        void _endExecution();
        void _handleTagEntry(const TagEntry& t);
        void _jumpToBlock(BlockId target);

        inline void _blockExecution();
    };
}
