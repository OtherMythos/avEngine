#pragma once

#if !defined(TARGET_APPLE_IPHONE) && !defined(TARGET_ANDROID)

#include "Window/Window.h"

#include "Window/SDL2Window/SDL2InputMapper.h"

namespace AV {
    class InputManager;
    class GuiInputProcessor;

    /**
     A window which never touches SDL or the windowing system at all.

     Enabled with the --headless command line flag. Nothing is created on screen and no
     SDL subsystem is initialised, so an engine run leaves no trace on the user's desktop.
     This is intended for agent driven integration tests and CI, where a window flashing
     up for the duration of a run is at best a distraction.

     Rendering still happens for real. Ogre needs exactly one Window object in order to
     finish initialising the render system and to create the CompositorManager2, so a
     bootstrap Ogre window is still created (hidden, or entirely windowless on backends
     which support it). That bootstrap window is never attached to a workspace and so is
     never presented. Everything is instead drawn into mRenderTexture, which the debug
     server reads back through Window::getRenderTexture.
     */
    class HeadlessWindow : public Window{
    public:
        HeadlessWindow();
        virtual ~HeadlessWindow();

        void update() override;
        bool initialise() override;
        bool open(InputManager* inputMan, GuiInputProcessor* guiInputProcessor) override;
        bool close() override;

        bool isInitialised() override;
        bool isOpen() override;

        void injectOgreWindow(Ogre::Window *window) override;

        bool setFullscreen(FullscreenMode fullscreen) override;
        bool setBorderless(bool enable) override;
        void setSize(int width, int height) override;

        int getWindowDisplayIndex() override;

        void rumbleInputDevice(InputDeviceId device, float lowFreqStrength, float highFreqStrength, uint32 rumbleTimeMs) override;

        bool showMessageBox(const MessageBoxData& msgData, int* pressedButton) override;

        Ogre::TextureGpu* getRenderTexture() const override { return mRenderTexture; }

        /**
         Supply the offscreen texture the compositor renders into.
         Called by OgreSetup once the render system exists, as the texture cannot be
         created before then.
         */
        void _setRenderTexture(Ogre::TextureGpu* texture) { mRenderTexture = texture; }

    protected:
        InputMapper* getInputMapper() override{
            return &inputMapper;
        }

    private:
        //SDL2InputMapper only ever references SDL's key and controller constants, never
        //the SDL runtime, so it works unchanged with no SDL subsystem initialised. Reusing
        //it means scripts map inputs identically headless and windowed, and the debug
        //server's input playback (which injects at the action level) just works.
        SDL2InputMapper inputMapper;

        GuiInputProcessor* mGuiInputProcessor;
        InputManager* mInputManager;

        //Owned by Ogre's TextureGpuManager, not by this class. See close().
        Ogre::TextureGpu* mRenderTexture;
    };
}

#endif
