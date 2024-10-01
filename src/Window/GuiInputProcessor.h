#pragma once

#include "Input/InputPrerequisites.h"

namespace AV{
    class GuiManager;
    class InputMapper;

    /**
    A class which interprets input and passes it onto the gui system.
    This is separate from the InputMapper, which is responsible for mapping actions to input, and has nothing to do with gui.
    */
    class GuiInputProcessor{
    public:
        GuiInputProcessor();
        ~GuiInputProcessor();

        void initialise(GuiManager* guiManager);

        /**
        Process a mouse move.

        @param x
        x value between 0 and 1 for the window.

        @param y
        y value between 0 and 1 for the window.
        */
        void processMouseMove(float x, float y);
        /**
        Process a mouse move in absolute coordinates.

        @param x
        x value expected to be between 0 and canvas width.

        @param y
        y value expected to be between 0 and canvas height.
        */
        void processMouseMoveAbsolute(float x, float y);
        /**
        Process a mouse button press.

        @returns true if that button press intercepted a gui element.
        */
        bool processMouseButton(int mouseButton, bool pressed);

        void processWindowResize(int width, int height);
        void processMouseScroll(float x, float y);

        void processTextInput(const char* text);
        void processTextEdit(const char *text, int selectStart, int selectLength);
        void processInputKey(const InputMapper& mapper,bool pressed, int scancode, int key, int keyMod, bool textInputEnabled);
        void processControllerButton(const InputMapper& mapper, bool pressed, int button);
        void processControllerAxis(const InputMapper& mapper, int axisId, float value, bool xAxis);

        /**
        Checks whether the gui system requests keyboard input.

        @returns
        True if it should be enabled, false otherwise.
        */
        bool shouldTextInputEnable();

    private:
        GuiManager* mGuiManager = 0;

        GuiInputTypes mCurrentControllerGuiType;
        float mAxisX, mAxisY;
    };
}
