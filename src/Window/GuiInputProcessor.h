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

    private:
        GuiManager* mGuiManager = 0;
    };
}
