#pragma once

namespace AV{
    /**
    A singleton class to manage input.

    This is NOT a permanent class. It is intended to be used for testing in the early stages.
    Input will later be handled by some rewrite of this system that takes into account things like gui and so on.
    It might stick around for a year or so, but hopefully not much longer.
    */
    class Input{
    public:
        static const int numKeys = 6;
        static const int numMouseButtons = 3;
        enum Input_Key{
            Key_Null,
            Key_Accept,
            Key_Decline,
            Key_Up,
            Key_Down,
            Key_Left,
            Key_Right,
        };


        static void setKeyActive(Input_Key key, bool active);
        static bool getKey(Input_Key key);

        static void setMouseX(int x) {
            mouseX = x;
        }

        static void setMouseY(int y) {
            mouseY = y;
        }

        static void setMouseWheel(int val) {
            mouseWheel = val;
        }

        static int getMouseX() { return mouseX; }
        static int getMouseY() { return mouseY; }
        static int getMouseWheel() { return mouseWheel; }

        static void setMouseButton(int mouseButton, bool pressed);
        static int getMouseButton(int mouseButton);

        static bool mouseButtons[numMouseButtons];

    private:
        static bool keys[numKeys];

        static int mouseX, mouseY, mouseWheel;
    };
}
