#pragma once

namespace AV{
    class GuiManager;

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
        void processMouseButton(int mouseButton, bool pressed);

        void processWindowResize(int width, int height);

        void processTextInput(const char* text);
        void processTextEdit(const char *text, int selectStart, int selectLength);
        void processInputKey(bool pressed, int key, int keyMod);

        /**
        Checks whether the gui system requests keyboard input.

        @returns
        True if it should be enabled, false otherwise.
        */
        bool shouldTextInputEnable();

    private:
        GuiManager* mGuiManager = 0;
    };
}
