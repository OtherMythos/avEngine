#pragma once

namespace AV{

    typedef unsigned int ActionHandle;
    typedef unsigned int InputDeviceId;

    /**
    A class to manage input in the engine.
    This class is responsible for abstracting much of the details of the input method.

    For instance, keyboards and game controllers have their input unified into 'actions'.
    Actions are context specific, as well as user customisable.
    This input system is largely based off of the steam input api.
    The main reason for this is to generally unify the programming concepts to avoid implementation issues later on.
    */
    class InputManager{
    public:
        InputManager();
        ~InputManager();

        /**
        Add an input device to the input manager.
        This should be called when a new controller is plugged into the pc/console/whatever.

        @returns
        True or false depending on whether the addition was successful or not.
        A maximum of 4 devices are allowed at once, so if trying to add a fifth one (for instance), false would be returned.
        */
        bool addInputDevice(InputDeviceId dev, const char* deviceName);

        bool removeInputDevice(InputDeviceId dev);

    private:
        static const char MAX_INPUT_DEVICES = 4;

        struct InputDeviceData{
            bool populated;
            char deviceName[20];
            bool buttonPressed; //Dummy value
        };

        InputDeviceData mDevices[MAX_INPUT_DEVICES];

        inline void _resetDeviceData(InputDeviceData& d) const;
    };
}
