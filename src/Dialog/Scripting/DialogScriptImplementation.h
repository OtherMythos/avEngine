#pragma once

#include <string>
#include <memory>
#include "Dialog/Compiler/DialogScriptData.h"

namespace AV{
    class CallbackScript;
    struct Entry4;
    struct Entry2;

    /**
    A class to manage the squirrel implementation of the dialog system.

    The dialog system is built in a way where the emphasis of its implementation is created through scripts.
    A script is set, which contains the actual implementation within its callbacks.
    The functions are then called during various events, and from there different implementations can be created.
    With this system practically any sort of interface, functioanlity, or use case could be fulfilled with this dialog system.
    */
    class DialogScriptImplementation{
    public:
        DialogScriptImplementation();
        ~DialogScriptImplementation();

        void initialise();

        void notifyDialogString(const std::string& str);
        void notifyDialogExecutionBegin();
        void notifyDialogExecutionEnded();

        void notifyActorMoveTo(const Entry4& e);
        void notifyActorChangeDirection(const Entry2& e);
        void notifyShouldCloseDialog();

        void notifyOption(const std::string** strings);

        void hideDialogWindow();

        bool isSetupCorrectly() const { return mSetupCorrectly; }

    private:
        std::shared_ptr<CallbackScript> mScript;
        bool mSetupCorrectly = false;

        int FIDdialogString = 0;
        int FIDdialogBegin = 0;
        int FIDdialogEnded = 0;
        int FIDactorMoveTo = 0;
        int FIDactorChangeDirection = 0;
        int FIDHideDialogWindow = 0;
        int FIDShouldCloseDialogWindow = 0;
        int FIDdialogOption = 0;
    };
}
