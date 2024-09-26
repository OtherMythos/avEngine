#pragma once

#include "Compiler/DialogScriptData.h"
#include <memory>
#include <chrono>
#include "System/Registry/ValueRegistry.h"
#include "System/Util/IdString.h"

namespace AV{
    class DialogScriptImplementation;
    class ValueRegistry;
    class CallbackScript;

    /**
    Process and manage running instances of dialog.
    The dialog system allows lots of functionality to be implemented into projects.
    The way in which dialog is implemented is left up to the user through a script interface.
    This class manages the creation, lifetime and execution of dialog objects.
    */
    class DialogManager{
    public:
        DialogManager();
        ~DialogManager();

        void shutdown();

        /**
        Begin the execution of a compiled dialog script.
        */
        bool beginExecution(const CompiledDialog& d = EMPTY_DIALOG, BlockId startBlock = 0);
        void setCompiledDialog(const CompiledDialog& d);
        void unsetCompiledDialog();

        /**
        Convenience function to compile a dialog script and execute it.
        Once the script has finished the compiled version will be destroyed automatically.

        @returns
        True or false depending on whether the dialog could be sucessfully compiled or not.
        */
        bool compileAndRunDialog(const std::string& path);

        /**
        Unblock the dialog execution.
        Certain dialog blocks cause the dialog to block itself, for instance waiting for the user to read something.
        This function tells the dialog manager to unblock itself, for instance if the player has pressed a button to continue the dialog.
        */
        void unblock();

        /**
        Alert the dialog manager of a compiled dialog that has recently been deleted.
        In the case of that dialog being currently set it will be unset, to avoid memory errors.
        This is generally intended for scripts. C++ users are expected to be able to manage their own dialogs.
        */
        bool notifyDialogDeletion(CompiledDialog* dialog);

        enum class DialogSpecifyOptionErrors{
            INVALID_ID,
            OPTION_NOT_ACTIVE,
            INVALID_TARGET,
            SUCCESS
        };
        /**
        Specify an option as part of the option tag.
        @returns SUCCESS if the option was received. This function can only be called while the dialog is blocked on an option.
        Returns a failure value otherwise.
        */
        DialogSpecifyOptionErrors specifyOption(int option);

        void update();

        void initialise();

        bool isExecuting() const { return mExecuting; }
        bool isBlocked() const { return mBlocked; }

        std::shared_ptr<ValueRegistry> getLocalRegistry() { return mLocalRegistry; }

        struct VariableCharContents{
            AttributeType type;
            bool isGlobal;
            bool isVariable;
        };
        static void _readVariableChar(char c, VariableCharContents& out);

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

        bool mRequestedDialogClose = false;

        std::chrono::steady_clock::time_point sleepBeginTime;

        bool mDestroyCompiledDialogOnEnd = false;

        BlockId mExecutingBlock = 0;
        int mExecTagIndex = 0;

        void _executeDialog();
        void _endExecution();
        bool _handleTagEntry(const TagEntry& t);
        bool _jumpToBlock(BlockId target);
        void _beginSleep(int milliseconds);
        bool _checkSleepInterval();
        void _notifyHideDialog();
        bool _executeScriptTag(int scriptIdx, const std::string& funcName, int variablesId, int totalVariables);

        /**
        Check prerequisites for running the dialog.
        For instance, creating script files at the beginning.

        @returns true or false depending on whether the system encountered an error.
        */
        bool _checkDialogPrerequisites();

        void _printErrorMessage();
        //Used to store information about the current runtime error.
        //Generally std strings are slower, especially if this is going into the release build, but the chance of a runtime error is low.
        //This will make it easier to create strings with variables in for instance, so it's worth the unlikely performance hit.
        std::vector<std::string> mErrorReason;

        int mSleepInterval = -1;

        inline void _blockExecution();

        inline std::shared_ptr<ValueRegistry> _getRegistry(bool registry);

        void _readIntVariable(int& out, const VariableAttribute& e, bool& outVal, TagType t, const char* attribName);
        void _readStringVariable(std::string& out, const VariableAttribute& e, bool& outVal, TagType t, const char* attribName);
        void _readBoolVariable(bool& out, const VariableAttribute& e, bool& outVal, TagType t, const char* attribName);


        template <class T>
        void _readVariable(RegistryLookup(ValueRegistry::*funcPtr)(IdString, T&), T& out, const VariableAttribute& e, bool& outVal, TagType t, const char* attribName, int* stringId = 0, bool* isConstant = 0);

        std::string _produceDialogVariableString(const std::string& initString, const std::string& replaceString);
        std::string _determineStringVariable(const std::string& str, std::string::const_iterator f, std::string::const_iterator s, bool globalVariable);

        std::map<int, std::shared_ptr<CallbackScript>> mDialogScripts;

        bool mOptionActive = false;
        int mDialogOptionTargets[4];
    };
}
