#include "HeadlessWindow.h"

#if !defined(TARGET_APPLE_IPHONE) && !defined(TARGET_ANDROID)

#include "Logger/Log.h"
#include "System/SystemSetup/SystemSettings.h"

#include "Event/Events/SystemEvent.h"
#include "Event/EventDispatcher.h"
#include "Window/GuiInputProcessor.h"

#include "OgreTextureGpu.h"

#ifdef _WIN32
    #include <OgreWindowEventUtilities.h>
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #include <crtdbg.h>
    #include <cstdlib>
#endif

namespace AV {
    HeadlessWindow::HeadlessWindow()
    : mGuiInputProcessor(0),
      mInputManager(0),
      mRenderTexture(0){

    }

    HeadlessWindow::~HeadlessWindow(){
        //Nothing to release here. See close().
    }

    bool HeadlessWindow::initialise(){
#ifdef _WIN32
        //The debug runtime raises a modal dialog for a failed CRT assertion or an abort(), and
        //Windows Error Reporting does the same for a crash. On a desktop nobody dismisses it, so the
        //process hangs forever. Report to stderr instead, so the run fails rather than stalls.
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
        _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
        _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
        _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
        SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#endif

        //Deliberately no SDL_Init. Skipping the video subsystem is the whole point of
        //this window; on macOS in particular that's what stops the process becoming a
        //foreground app with a dock icon.
        _width = SystemSettings::getDefaultWindowWidth();
        _height = SystemSettings::getDefaultWindowHeight();
        _currentTitle = getDefaultWindowName();

        _initialised = true;
        return true;
    }

    bool HeadlessWindow::open(InputManager* inputMan, GuiInputProcessor* guiInputProcessor, InputRouter* inputRouter){
        mGuiInputProcessor = guiInputProcessor;
        if(isOpen() || !isInitialised()){
            return false;
        }

        _open = true;

        //Read again here rather than relying on initialise(), as the Squirrel setup() function runs
        //between the two and can change the intended size, which SDL2Window also reads in open().
        _width = SystemSettings::getDefaultWindowWidth();
        _height = SystemSettings::getDefaultWindowHeight();

        inputMapper.initialise(inputMan);
        if(SystemSettings::getUseDefaultActionSet()) inputMapper.setupMap();
        mInputManager = inputMan;

        return true;
    }

    bool HeadlessWindow::close(){
        if(!isOpen()){
            return false;
        }
        _open = false;

        //Both of these belong to Ogre, and Base::shutdown deletes the root before calling
        //close(), which already destroyed them. Just drop the dangling pointers.
        mRenderTexture = 0;
        _ogreWindow = 0;

        return true;
    }

    bool HeadlessWindow::isInitialised(){
        return _initialised;
    }

    bool HeadlessWindow::isOpen(){
        return _open;
    }

    void HeadlessWindow::update(){
        //There is no event queue to pump. The only exception is Windows, where the
        //bootstrap window may be a real (hidden) HWND; leaving its message queue
        //unserviced makes the OS mark the process as not responding.
        #ifdef _WIN32
            Ogre::WindowEventUtilities::messagePump();
        #endif
    }

    void HeadlessWindow::injectOgreWindow(Ogre::Window *window){
        //Unlike SDL2Window there's deliberately no requestResolution here. This window is
        //only a bootstrap for the render system and is never rendered to, so its size is
        //irrelevant. The size that matters belongs to mRenderTexture.
        _ogreWindow = window;
    }

    bool HeadlessWindow::setFullscreen(FullscreenMode fullscreen){
        _fullscreen = (fullscreen != FullscreenMode::WINDOWED);
        return true;
    }

    bool HeadlessWindow::setBorderless(bool enable){
        _borderless = enable;
        return true;
    }

    void HeadlessWindow::setSize(int width, int height){
        if(width <= 0 || height <= 0) return;

        _width = width;
        _height = height;

        if(mRenderTexture){
            //A resident texture can't change resolution in place, so cycle it out to
            //storage and back. This is the same mechanism a real window resize uses.
            mRenderTexture->scheduleTransitionTo(Ogre::GpuResidency::OnStorage);
            mRenderTexture->setResolution(static_cast<Ogre::uint32>(_width), static_cast<Ogre::uint32>(_height));
            mRenderTexture->scheduleTransitionTo(Ogre::GpuResidency::Resident);
        }

        if(mGuiInputProcessor) mGuiInputProcessor->processWindowResize(_width, _height);

        SystemEventWindowResize e;
        e.width = _width;
        e.height = _height;

        EventDispatcher::transmitEvent(EventType::System, e);
    }

    int HeadlessWindow::getWindowDisplayIndex(){
        return -1;
    }

    void HeadlessWindow::rumbleInputDevice(InputDeviceId device, float lowFreqStrength, float highFreqStrength, uint32 rumbleTimeMs){
        //No devices are opened headless.
    }

    bool HeadlessWindow::showMessageBox(const MessageBoxData& msgData, int* pressedButton){
        //A modal dialog nobody can dismiss would hang the run.
        AV_WARN("Message box suppressed in headless mode. Title: '{}' Message: '{}'", msgData.title, msgData.message);

        if(pressedButton) *pressedButton = -1;
        return false;
    }
}

#endif
