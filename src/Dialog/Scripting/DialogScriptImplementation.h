#pragma once

namespace AV{
    class CallbackScript;

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

    private:
        CallbackScript *mScript;
    };
}
