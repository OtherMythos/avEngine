#pragma once

#include "InputPrerequisites.h"
#include "System/EnginePrerequisites.h"

#include <vector>
#include <string>
#include <map>

namespace AV{
    class InputMapper;

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
        Update the keys pressed time.
        */
        void update(float delta);

        /**
        Add an input device to the input manager.
        This should be called when a new controller is plugged into the pc/console/whatever.

        @returns
        An id representing the input device id.
        If valid this will be a number between 0 and MAX_INPUT_DEVICES.
        If an error occurred, INVALID_INPUT_DEVICE will be returned.
        A maximum of 4 devices are allowed at once, so if trying to add a fifth one (for instance), this error would be returned.
        */
        InputDeviceId addInputDevice(const char* deviceName);

        /**
        Remove an input device by id.
        This is intended to be called when the physical device is removed.
        */
        bool removeInputDevice(InputDeviceId dev);

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
        @return True if the deadzone was hit and the axis move was changed to 0.0, false otherwise.
        */
        bool setAxisAction(InputDeviceId id, ActionHandle action, bool x, float axis);
        /**
        Set the value for a trigger action.
        */
        void setAnalogTriggerAction(InputDeviceId id, ActionHandle action, float axis);

        void setKeyboardInput(int key, bool pressed);
        bool getKeyboardInput(int key);

        /**
        A function to facilitate the mapping of keyboard buttons to controller inputs.
        This allows the keyboard to emulate all types of controller inputs (button, trigger, stick).
        From the handle, the information regarding the set action can be found.
        @param value
        Multi function value depending on the action type.
        If passing a button acton, a float value of 0 will represent a release, and anything else will a press.
        If passing a trigger the value should be between 0 and 1.
        If passing an axis, the function will read the axis type from the handle, and use this float to set the correct axis value.
        */
        void setKeyboardKeyAction(ActionHandle action, float value);

        bool getButtonAction(InputDeviceId device, ActionHandle action, InputTypes input = INPUT_TYPE_ANY) const;
        float getTriggerAction(InputDeviceId id, ActionHandle action) const;
        float getAxisAction(InputDeviceId id, ActionHandle action, bool x) const;

        /**
        Create an action set which can be populated with values.
        */
        ActionSetHandle createActionSet(const char* actionSetName);
        void _pushNewAction();

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
        void createAction(const char* actionName, ActionSetHandle actionSet, ActionType type, bool firstValue);

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
                "DirectionUp"
                "DirectionDown"
                "DirectionLeft"
                "DirectionRight"
            }
        }

        This action set is used when the user doesn't formally define one.
        It will have its inputs mapped to sensible buttons for keyboards and controllers.
        */
        void setupDefaultActionSet();

        /**
        Get the device which was used last frame.
        Can be useful to determine which devices are actually being used to interact with the project.
        @returns The most recent device which was used.
        If multiple last frame, this will be in the order keyboard, then controllers 0-MAX_INPUT_DEVICES
        */
        char getMostRecentDevice() const;

        //Contains information about where to look in the mActionSetData list.
        //All sets are packed into the same list, so these indexes describe where each piece of data starts and finishes.
        struct ActionSetEntry{
            std::string actionSetName;

            size_t buttonStart;
            size_t buttonEnd; //End value is non inclusive

            size_t analogTriggerStart;
            size_t analogTriggerEnd;

            size_t stickStart;
            size_t stickEnd;
        };

        /**
        Call when a device changes its action set.
        This updates the necessary information.
        */
        void notifyDeviceChangedActionSet(InputMapper* mapper, ActionSetHandle newSet, ActionSetHandle oldSet, InputDeviceId device);

        //Name and the index position. In future this might also include the localised name or something.
        typedef std::pair<std::string, int> ActionSetDataEntry;

    private:

        int mNumActiveControllers = 0;
        struct InputDeviceData{
            bool populated;
            char deviceName[20];
            float deadzone;
        };

        //Lists the data for each action set. This data is a collection of where to look in the various lists for the correct data.
        //The user is expected to use handles for everything. So, you would first query the handle, store it somewhere, and use that to query input.
        //This approach means that these vectors and maps are only used when querying the handle or any other sort of metadata.
        //They're not expected to be referenced each frame.
        std::vector<ActionSetEntry> mActionSets;
        std::vector<ActionSetDataEntry> mActionSetData;

    public:

        //Actual place where input data is written to.
        //The handle system allows the ability to directly lookup these values.
        //TODO in future these are going to be inserted into lists per device.
        struct StickPadGyroData{
            float x;
            float y;
        };
        template <typename T>
        struct ActionData{
            std::vector<T> actionButtonData;
            std::vector<float> actionAnalogTriggerData;
            std::vector<StickPadGyroData> actionStickPadGyroData;
            //Keeps track of how long the action has been active.
            //-1.0f for not active, otherwise a count of seconds.
            std::vector<float> actionDuration;
            //Previous to keep track for action release.
            std::vector<float> actionDurationPrev;
        };

    private:
        ActionData<bool> mActionData[MAX_INPUT_DEVICES];
        ActionData<bool> mKeyboardData;

        std::vector<bool> mKeysPressed;

        /**
        Keep track of which devices were used this frame.
        The first bool designates whether any inputs were received.
        The second the keyboard device, the other indexes are the controller inputs.
        If multiple devices are used, the first in the list is returned as the most recent.
        */
        bool mMostRecentDevice[MAX_INPUT_DEVICES + 2];

        typedef unsigned char AnyButtonActionCounter;
        /**
        Special data specific to the any action device.
        It has to be separate as it's requirements are different.
        Buttons are stored as chars because it needs to keep track of how many devices have that button pressed, for instance.
        */
        ActionData<AnyButtonActionCounter> mAnyDeviceData;

        InputDeviceData mDevices[MAX_INPUT_DEVICES];

        inline void _resetDeviceData(InputDeviceData& d) const;

        ActionHandle _getActionHandle(ActionType type, const std::string& actionName);
        //Get the target axis from a handle. This is only uesd with keyboards. This is assuming that this handle was populated by the input mapper.
        int _getHandleAxis(ActionHandle action);
        inline void _printHandleError(const char* funcName) const;

        int mMouseX, mMouseY;
        int mActualMouseX, mActualMouseY;
        int mMouseWheel;
        bool mMouseGuiIntersected;
        float mDefaultAxisDeadZone;

        static const int NUM_MOUSE_BUTTONS = 3;
        bool mMouseButtons[NUM_MOUSE_BUTTONS];
        bool mMousePressed[NUM_MOUSE_BUTTONS];
        bool mMouseReleased[NUM_MOUSE_BUTTONS];


        struct TouchData{
            float x, y;
        };
        std::map<uint64, TouchData> mTouchData;


    public:
        const std::vector<ActionSetEntry>& getActionSets() const { return mActionSets; }
        const std::vector<ActionSetDataEntry>& getActionSetData() const { return mActionSetData; }

        struct ActionHandleContents{
            ActionType type;
            unsigned char itemIdx;
            unsigned char actionSetId;
        };

        ActionHandle _produceActionHandle(const ActionHandleContents& contents) const;
        void _readActionHandle(ActionHandleContents* outContents, ActionHandle handle) const;

        int getNumberOfActiveControllers() const { return mNumActiveControllers; }
        const char* getDeviceName(InputDeviceId id) const;
        int getNumActionSets() const { return static_cast<int>(mActionSets.size()); }

        int getMouseX() const { return mMouseX; }
        int getMouseY() const { return mMouseY; }
        void setMouseX(int x) { mMouseX = x; }
        void setMouseY(int y) { mMouseY = y; }

        void setAxisDeadzone(float deadzone, InputDeviceId device);
        float getAxisDeadzone(InputDeviceId device) const;
        void setDefaultAxisDeadzone(float deadzone) { mDefaultAxisDeadZone = deadzone; }

        int getActualMouseX() const { return mActualMouseX; }
        int getActualMouseY() const { return mActualMouseY; }
        void setActualMouseX(int x) { mActualMouseX = x; }
        void setActualMouseY(int y) { mActualMouseY = y; }

        void notifyTouchBegan(uint64 fingerId, float x, float y);
        void notifyTouchEnded(uint64 fingerId);
        void notifyTouchMotion(uint64 fingerId, float x, float y);

        bool getTouchPosition(uint64 fingerId, float* x, float* y);

        void setMouseWheel(int wheel) { mMouseWheel = wheel; }
        int getMouseWheel() const { return mMouseWheel; }

        void setMouseButton(int mouseButton, bool pressed, bool guiIntersected);
        bool getMouseButton(int mouseButton) const;
        bool getMousePressed(int mouseButton) const;
        bool getMouseReleased(int mouseButton) const;


        bool getMouseGuiIntersected() const { return mMouseGuiIntersected; }
    };
}
