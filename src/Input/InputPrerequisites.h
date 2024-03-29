#pragma once

namespace AV{
    typedef unsigned int ActionHandle;
    typedef unsigned char InputDeviceId;

    //Really this is nothing more than an index into the actionSetList.
    //I don't expect to get more than 256 action sets, so no point supporting it!
    typedef unsigned char ActionSetHandle;

    static const char MAX_INPUT_DEVICES = 4;
    static const char INVALID_INPUT_DEVICE = 30;
    static const char KEYBOARD_INPUT_DEVICE = 31;
    //All devices can set entries for this device.
    static const char ANY_INPUT_DEVICE = 32;

    static const ActionSetHandle INVALID_ACTION_SET_HANDLE = 0xff; //I don't expect to get more action sets than this.
    static const ActionHandle INVALID_ACTION_HANDLE = 0xffffffff;

    enum class ActionType{
        Unknown,
        StickPadGyro,
        AnalogTrigger,
        Button
    };

    enum InputTypes{
        INPUT_TYPE_ANY = 1u << 0,
        INPUT_TYPE_PRESSED = 1u << 1,
        INPUT_TYPE_RELEASED = 1u << 2,
    };

    enum class GuiInputTypes{
        None,
        Top,
        Left,
        Right,
        Bottom,
        Primary
    };

    struct GuiMappedAxisData{
        GuiInputTypes top;
        GuiInputTypes bottom;
        GuiInputTypes left;
        GuiInputTypes right;
    };
}
