#pragma once

#include "Compiler/DialogScriptData.h"
#include <memory>
#include <chrono>
#include "OgreIdString.h"
#include "System/Registry/ValueRegistry.h"

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

        std::shared_ptr<ValueRegistry> getLocalRegistry() { return mLocalRegistry; }

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
        bool _handleTagEntry(const TagEntry& t);
        void _jumpToBlock(BlockId target);
        void _beginSleep(int milliseconds);
        bool _checkSleepInterval();
        void _notifyHideDialog();
        void _executeScriptTag(int scriptIdx, const std::string& funcName);

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

        struct VariableCharContents{
            AttributeType type;
            bool isGlobal;
            bool isVariable;
        };
        void _readVariableChar(char c, VariableCharContents& out);

        template <class T>
        void _readVariable(RegistryLookup(ValueRegistry::*funcPtr)(Ogre::IdString, T&), T& out, const VariableAttribute& e, bool& outVal, TagType t, const char* attribName, int* stringId = 0, bool* isConstant = 0);

        std::string _produceDialogVariableString(const std::string& initString, const std::string& replaceString);
        std::string _determineStringVariable(const std::string& str, std::string::const_iterator f, std::string::const_iterator s, bool globalVariable);
    };
}
