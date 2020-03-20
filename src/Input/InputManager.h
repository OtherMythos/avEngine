#pragma once

#include <vector>
#include <map>

namespace AV{

    typedef unsigned int ActionHandle;
    typedef unsigned char InputDeviceId;

    //Really this is nothing more than an index into the actionSetList.
    //I don't expect to get more than 256 action sets, so no point supporting it!
    typedef unsigned char ActionSetHandle;

    static const char MAX_INPUT_DEVICES = 4;
    static const char INVALID_INPUT_DEVICE = 30;
    static const char KEYBOARD_INPUT_DEVICE = 31;

    static const ActionSetHandle INVALID_ACTION_SET_HANDLE = 0xff; //I don't expect to get more action sets than this.
    static const ActionHandle INVALID_ACTION_HANDLE = 0xffffffff;

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

        enum class ActionType{
            Unknown,
            StickPadGyro,
            AnalogTrigger,
            Button
        };

        /**
        Add an input device to the input manager.
        This should be called when a new controller is plugged into the pc/console/whatever.

        @returns
        An id representing the input device id.
        If valid this will be a number between 0 and MAX_INPUT_DEVICES.
        If an error occurred, INVALID_INPUT_DEVICE will be returned.
        A maximum of 4 devices are allowed at once, so if trying to add a fifth one (for instance), this error would be returned.
        */
        //bool addInputDevice(InputDeviceId dev, const char* deviceName);
        InputDeviceId addInputDevice(const char* deviceName);

        bool removeInputDevice(InputDeviceId dev);

        /**
        Set the current action set of the input manager.
        */
        void setCurrentActionSet(ActionSetHandle actionSet);

        /**
        Get the handle of an action set based on its name.

        @returns
        The handle of the action set if found. INVALID_ACTION_SET_HANDLE otherwise.
        */
        ActionSetHandle getActionSetHandle(const std::string& setName) const;

        /**
        Get the action handle for a digital action.
        A digital action can be thought of as simply a button (on or off).
        This function will search all the action sets for an action with this name id, and return its handle.
        */
        ActionHandle getDigitalActionHandle(const std::string& actionName);

        /**
        Set the value of a digital action. This function is intended to be called by a component which received hardware inputs and converts them into actions.
        */
        void setDigitalAction(InputDeviceId device, ActionHandle action, bool val);

        bool getDigitalAction(InputDeviceId device, ActionHandle action) const;

        ActionSetHandle createActionSet(const char* actionSetName);
        void createAction(const char* actionName, ActionSetHandle actionSet, ActionType type);

    private:

        struct InputDeviceData{
            bool populated;
            char deviceName[20];
            bool buttonPressed; //Dummy value
        };

        //Contains information about where to look in the mActionSetData list.
        //All sets are packed into the same list, so these indexes describe where each piece of data starts and finishes.
        struct ActionSetEntry{
            size_t buttonStart;
            size_t buttonEnd;
        };

        //Name and the index position. In future this might also include the localised name or something.
        typedef std::pair<std::string, int> ActionSetDataEntry;

        //Lists the data for each action set. This data is a collection of where to look in the various lists for the correct data.
        //The user is expected to use handles for everything. So, you would first query the handle, store it somewhere, and use that to query input.
        //This approach means that these vectors and maps are only used when querying the handle or any other sort of metadata.
        //They're not expected to be referenced each frame.
        std::vector<ActionSetEntry> mActionSets;
        std::vector<ActionSetDataEntry> mActionSetData;
        std::map<std::string, ActionSetHandle> mActionSetMeta; //TODO I might be able to just have the string in the ActionSetEntry struct struct. This could then be removed.
        ActionSetHandle mCurrentActionSet = INVALID_ACTION_SET_HANDLE;

        //Actual place where input data is written to.
        //Then handle system allows the ability to directly lookup these values.
        std::vector<bool> mActionButtonData;

        InputDeviceData mDevices[MAX_INPUT_DEVICES];

        inline void _resetDeviceData(InputDeviceData& d) const;

        struct ActionHandleContents{
            ActionType type;
            unsigned char itemIdx;
            unsigned char actionSetId;
        };

        ActionHandle _produceActionHandle(const ActionHandleContents& contents) const;
        void _readActionHandle(ActionHandleContents* outContents, ActionHandle handle) const;
    };
}
