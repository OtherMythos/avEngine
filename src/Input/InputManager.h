#pragma once

#include "InputPrerequisites.h"

#include <vector>
#include <map>

namespace AV{

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
        Get the action handle for a button action.
        A button can be simply on or off.
        This function will search all the action sets for an action with this name id, and return its handle.
        */
        ActionHandle getButtonActionHandle(const std::string& actionName);
        ActionHandle getAxisActionHandle(const std::string& actionName);
        ActionHandle getAnalogTriggerActionHandle(const std::string& actionName);

        /**
        Set the value of a button action. This function is intended to be called by a component which received hardware inputs and converts them into actions.
        */
        void setButtonAction(InputDeviceId device, ActionHandle action, bool val);
        /**
        Set the value for an axis action.
        @param x
        A boolean representing whether this set is for the x axis. If false it will be assumed to be the y.
        */
        void setAxisAction(InputDeviceId id, ActionHandle action, bool x, float axis);

        void setAnalogTriggerAction(InputDeviceId id, ActionHandle action, float axis);

        bool getButtonAction(InputDeviceId device, ActionHandle action) const;
        float getTriggerAction(InputDeviceId id, ActionHandle action) const;
        float getAxisAction(InputDeviceId id, ActionHandle action, bool x) const;

        ActionSetHandle createActionSet(const char* actionSetName);

        /**
        Create an action within an action set.
        This function is intended to be called in batches, i.e iterating a list of actions and inserting them in one at a time.
        You shouldn't for instance, insert from one action set and then another. Doing this will cause the various internal numbers to get out of sync.

        @param actionName
        The name of the action to create. This name should be unique, even among other action sets.
        @param actionSet
        The handle of the target action set.
        @param type
        The type of action to create. Unknown should not be passed to this function.
        @param firstValue
        Whether this is the first value being inserted in this batch of actions.
        */
        size_t createAction(const char* actionName, ActionSetHandle actionSet, ActionType type, bool firstValue);

        /**
        Destroy all action sets and actions.
        */
        void clearAllActionSets();

        /**
        Setup the default engine action set. This looks like:
        Default{
            StickPadGyro{
                "LeftMove"
                "RightMove"
            }
            AnalogTrigger{
                "LeftTrigger"
                "RightTrigger"
            }
            Buttons{
                "Accept" //A
                "Decline" //B
                "Menu" //X
                "Options" //Y
                "Start"
                "Select"
            }
        }
        */
        void setupDefaultActionSet();

        //Contains information about where to look in the mActionSetData list.
        //All sets are packed into the same list, so these indexes describe where each piece of data starts and finishes.
        struct ActionSetEntry{
            size_t buttonStart;
            size_t buttonEnd; //End value is non inclusive

            size_t analogTriggerStart;
            size_t analogTriggerEnd;

            size_t stickStart;
            size_t stickEnd;
        };

        //Name and the index position. In future this might also include the localised name or something.
        typedef std::pair<std::string, int> ActionSetDataEntry;

    private:

        int mNumActiveControllers = 0;
        struct InputDeviceData{
            bool populated;
            char deviceName[20];
            bool buttonPressed; //Dummy value
        };

        //Lists the data for each action set. This data is a collection of where to look in the various lists for the correct data.
        //The user is expected to use handles for everything. So, you would first query the handle, store it somewhere, and use that to query input.
        //This approach means that these vectors and maps are only used when querying the handle or any other sort of metadata.
        //They're not expected to be referenced each frame.
        std::vector<ActionSetEntry> mActionSets;
        std::vector<ActionSetDataEntry> mActionSetData;
        std::map<std::string, ActionSetHandle> mActionSetMeta; //TODO I might be able to just have the string in the ActionSetEntry struct struct. This could then be removed.
        ActionSetHandle mCurrentActionSet = INVALID_ACTION_SET_HANDLE;

        //Actual place where input data is written to.
        //The handle system allows the ability to directly lookup these values.
        //TODO in future these are going to be inserted into lists per device.
        struct StickPadGyroData{
            float x;
            float y;
        };
        struct ActionData{
            std::vector<bool> actionButtonData;
            std::vector<float> actionAnalogTriggerData;
            std::vector<StickPadGyroData> actionStickPadGyroData;
        };
        ActionData mActionData[MAX_INPUT_DEVICES];

        InputDeviceData mDevices[MAX_INPUT_DEVICES];

        inline void _resetDeviceData(InputDeviceData& d) const;

        struct ActionHandleContents{
            ActionType type;
            unsigned char itemIdx;
            unsigned char actionSetId;
        };

        ActionHandle _produceActionHandle(const ActionHandleContents& contents) const;
        void _readActionHandle(ActionHandleContents* outContents, ActionHandle handle) const;

        ActionHandle _getActionHandle(ActionType type, const std::string& actionName);
        inline void _printHandleError(const char* funcName) const;

    public:
        const std::map<std::string, ActionSetHandle>& getActionSetMeta() const { return mActionSetMeta; }
        const std::vector<ActionSetEntry>& getActionSets() const { return mActionSets; }
        const std::vector<ActionSetDataEntry>& getActionSetData() const { return mActionSetData; }

        int getNumberOfActiveControllers() const { return mNumActiveControllers; }
        const char* getDeviceName(InputDeviceId id) const;
        int getNumActionSets() const { return mActionSets.size(); }
    };
}
