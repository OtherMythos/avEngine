#pragma once

#include "Compiler/DialogScriptData.h"
#include <memory>
#include <chrono>

namespace AV{
    class DialogScriptImplementation;
    class ValueRegistry;

    class DialogManager{
    public:
        DialogManager();
        ~DialogManager();

        void shutdown();

        /**
        Begin the execution of a compiled dialog script.
        */
        void beginExecution(const CompiledDialog& d = EMPTY_DIALOG, BlockId startBlock = 0);
        void setCompiledDialog(const CompiledDialog& d);
        void unsetCompiledDialog();

        /**
        Convenience function to compile a dialog script and execute it.
        Once the script has finished the compiled version will be destroyed automatically.

        @returns
        True or false depending on whether the dialog could be sucessfully compiled or not.
        */
        bool compileAndRunDialog(const std::string& path);

        //Unblock the dialog execution.
        void unblock();

        /**
        Alert the dialog manager of a compiled dialog that has recently been deleted.
        In the case of that dialog being currently set it will be unset, to avoid memory errors.
        This is generally intended for scripts. C++ users are expected to be able to manage their own dialogs.
        */
        bool notifyDialogDeletion(CompiledDialog* dialog);

        void update();

        void initialise();

        bool isExecuting() const { return mExecuting; }
        bool isBlocked() const { return mBlocked; }

    private:
        std::shared_ptr<DialogScriptImplementation> mImplementation;
        std::shared_ptr<ValueRegistry> mLocalRegistry;
        CompiledDialog mCurrentDialog;

        //Whether a compiled dialog script has actually been set.
        bool mDialogSet = false;
        //Whether a dialog script is being run at the moment.
        bool mExecuting = false;
        //Whether the current execution of the script has been blocked.
        bool mBlocked = false;
        bool mSleeping = false;

        bool mRequestedDiaogClose = false;

        std::chrono::steady_clock::time_point sleepBeginTime;

        bool mDestroyCompiledDialogOnEnd = false;

        BlockId mExecutingBlock = 0;
        int mExecTagIndex = 0;

        void _executeDialog();
        void _endExecution();
        void _handleTagEntry(const TagEntry& t);
        void _jumpToBlock(BlockId target);
        void _beginSleep(int milliseconds);
        bool _checkSleepInterval();
        void _notifyHideDialog();

        int mSleepInterval = -1;

        inline void _blockExecution();

        std::string _produceDialogVariableString(const std::string& initString, const std::string& replaceString);
        std::string _determineStringVariable(const std::string& str, std::string::const_iterator f, std::string::const_iterator s, bool globalVariable);
    };
}
