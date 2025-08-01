#include "SDL2Window.h"

#include "Logger/Log.h"
#include "System/SystemSetup/SystemSettings.h"
#include "OgreWindow.h"
#include "OgreStringConverter.h"

#include "Event/Events/SystemEvent.h"
#include "Event/EventDispatcher.h"

#include <OgreWindowEventUtilities.h>

#include "System/BaseSingleton.h"
#include "Gui/GuiManager.h"

#include <iostream>
#include <SDL.h>
#include <SDL_syswm.h>

#include <SDL_system.h>

#include "Input/InputManager.h"

#include "System/Base.h"

#include "Event/Events/DebuggerToolEvent.h"

#include "SDL_gamecontroller.h"

#ifdef __APPLE__
    #include "MacOS/MacOSUtils.h"
#endif


namespace AV {
    SDL2Window::SDL2Window()
        : mResetInputsAtFrameEnd(false) {

        _open = false;
        _initialised = false;

        SDL_version compiled;
        SDL_version linked;
        SDL_VERSION(&compiled);
        SDL_GetVersion(&linked);
        AV_INFO("Built against SDL version {}.{}.{}", compiled.major, compiled.minor, compiled.patch);
        AV_INFO("Linking against SDL version {}.{}.{}", linked.major, linked.minor, linked.patch);

        for(int i = 0; i < MAX_INPUT_DEVICES; i++){
            mOpenGameControllers[i] = {0, 0};
        }
    }

    SDL2Window::~SDL2Window(){

    }

    void SDL2Window::update(){
        mInputManager->setMouseWheel(0);

        bool shouldTextInput = mGuiInputProcessor->shouldTextInputEnable();
        if(shouldTextInput && !isKeyboardInputEnabled){
            isKeyboardInputEnabled = true;
            SDL_StartTextInput();
            AV_INFO("Enabling text input");
        }
        else if(!shouldTextInput && isKeyboardInputEnabled){
            isKeyboardInputEnabled = false;
            SDL_StopTextInput();
            AV_INFO("Disabling text input");
        }

        Ogre::WindowEventUtilities::messagePump();
        SDL_PumpEvents();

        _pollForEvents();

        if(mResetInputsAtFrameEnd){
            //Calling setFullscreen from a button press would wipe sdl events,
            //meaning mouse the mouse button was always down.
            _handleMouseButton(0, false);
            _handleMouseButton(1, false);
            _handleMouseButton(2, false);
            mResetInputsAtFrameEnd = false;
        }
    }

    uint32 _getFullscreenFlag(FullscreenMode fullscreen){
        uint32 flag = 0;
        if(fullscreen == FullscreenMode::FULLSCREEN){
            flag = SDL_WINDOW_FULLSCREEN;
        }
        else if(fullscreen == FullscreenMode::FULLSCREEN_BORDERLESS){
            flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
        }else{
            flag = 0;
        }
        return flag;
    }

    bool SDL2Window::initialise(){
        if(isInitialised()){
            return false;
        }

        SDL_SetHint(SDL_HINT_WINDOWS_DPI_SCALING, "1");
        if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER) < 0){
            return false;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        _initialised = true;

        return true;
    }

    bool SDL2Window::open(InputManager* inputMan, GuiInputProcessor* guiManager){
        mGuiInputProcessor = guiManager;
        if(isOpen() || !isInitialised()){
            //If the window is already open don't open it again.
            return false;
        }

        uint32 fullscreenFlag = _getFullscreenFlag(SystemSettings::getDefaultFullscreenMode());

        Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | fullscreenFlag;
        if(SystemSettings::isWindowResizable()){
            flags |= SDL_WINDOW_RESIZABLE;
        }

        #if defined(TARGET_APPLE_IPHONE) || defined(TARGET_ANDROID)
            flags |= SDL_WINDOW_FULLSCREEN;
            SDL_SetEventFilter(_handleAppEvents, NULL);

            SDL_Rect rec;
            SDL_GetDisplayBounds(0, &rec);

            SystemSettings::setDefaultWidth(rec.w);
            SystemSettings::setDefaultHeight(rec.h);

            const char* error = SDL_GetError();
        #endif

        _width = SystemSettings::getDefaultWindowWidth();
        _height = SystemSettings::getDefaultWindowHeight();

        _SDLWindow = SDL_CreateWindow(getDefaultWindowName().c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _width, _height, flags);

        #ifndef TARGET_APPLE_IPHONE
        if(SystemSettings::getDefaultFullscreenMode() != FullscreenMode::WINDOWED){
            int w, h;
            SDL_GL_GetDrawableSize(_SDLWindow, &w, &h);
            SystemSettings::setDefaultWidth(w);
            SystemSettings::setDefaultHeight(h);
            _width = SystemSettings::getDefaultWindowWidth();
            _height = SystemSettings::getDefaultWindowHeight();
        }
        #endif

        #ifdef TARGET_APPLE_IPHONE
            //I found this was needed to get SDL to process touch events.
            SDL_Renderer* renderer = SDL_CreateRenderer(_SDLWindow, 0, 0);
            SDL_RenderClear(renderer);
            SDL_RenderPresent(renderer);
            SDL_DestroyRenderer(renderer);
        #endif

        _open = true;
        inputMapper.initialise(inputMan);
        if(SystemSettings::getUseDefaultActionSet()) inputMapper.setupMap();
        mInputManager = inputMan;

        SDL_StopTextInput(); //Turn this off by default.
        isKeyboardInputEnabled = false;

        _setupSystemCursors();

        return true;
    }

    bool SDL2Window::close(){
        if(!isOpen()){
            return false;
        }

        for(int i = 0; i < MAX_INPUT_DEVICES; i++){
            if(!mOpenGameControllers[i].controller) continue;
            SDL_GameControllerClose(mOpenGameControllers[i].controller);
        }

        _open = false;
        //The Ogre window should've already been destroyed by shutdown of Ogre root.
        //_ogreWindow->destroy();

        _destroySystemCursors();

        SDL_DestroyWindow(_SDLWindow);
        SDL_Quit();

        return true;
    }

    bool SDL2Window::setFullscreen(FullscreenMode fullscreen){
        #if defined(TARGET_APPLE_IPHONE) || defined(TARGET_ANDROID)
            //Mobile is always fullscreen.
            return true;
        #endif

        uint32 flag = _getFullscreenFlag(fullscreen);

        bool result = (SDL_SetWindowFullscreen(_SDLWindow, flag) == 0);

        //Reset all inputs when switching to fullscreen.
        mResetInputsAtFrameEnd = true;

        return result;
    }

    bool SDL2Window::setBorderless(bool borderless){
        #if defined(TARGET_APPLE_IPHONE) || defined(TARGET_ANDROID)
            //Mobile cannot have borderless set.
            return false;
        #endif

        _borderless = borderless;

        SDL_bool enabled = !borderless ? SDL_TRUE : SDL_FALSE;
        SDL_SetWindowBordered(_SDLWindow, enabled);

        return true;
    }

    void SDL2Window::_setSize(int width, int height){
        _width = width;
        _height = height;

        if (_ogreWindow) {
#ifdef _WIN32
            _ogreWindow->windowMovedOrResized();
#else
            _ogreWindow->requestResolution(_width, _height);
#endif
        }

        mGuiInputProcessor->processWindowResize(_width, _height);

        SystemEventWindowResize e;
        e.width = _width;
        e.height = _height;

        EventDispatcher::transmitEvent(EventType::System, e);
    }
    void SDL2Window::setSize(int width, int height){
        SDL_SetWindowSize(_SDLWindow, width, height);
        _setSize(width, height);
    }

    int SDL2Window::getWindowDisplayIndex(){
        int index = SDL_GetWindowDisplayIndex(_SDLWindow);

        return index;
    }

    void SDL2Window::_handleBasicWindowEvent(const SDL_WindowEvent& event){
        SystemEvent* e;

        switch(event.event){
            case SDL_WINDOWEVENT_MINIMIZED: e = new SystemEventWindowMinimized(); break;
            case SDL_WINDOWEVENT_MAXIMIZED: e = new SystemEventWindowMaximised(); break;
            case SDL_WINDOWEVENT_FOCUS_GAINED: e = new SystemEventWindowFocusGained(); break;
            case SDL_WINDOWEVENT_FOCUS_LOST: e = new SystemEventWindowFocusLost(); break;
            case SDL_WINDOWEVENT_SHOWN: e = new SystemEventWindowShown(); break;
            case SDL_WINDOWEVENT_HIDDEN: e = new SystemEventWindowHidden(); break;
            case SDL_WINDOWEVENT_RESTORED: e = new SystemEventWindowRestored(); break;
            case SDL_WINDOWEVENT_EXPOSED: e = new SystemEventWindowExposed(); break;
            default:{
                //This function should not be called unless one of the above events occurs.
                assert(false);
            }
        }

        EventDispatcher::transmitEvent(EventType::System, *e);
        delete e;
    }

    int SDL2Window::_handleAppEvents(void *userdata, SDL_Event *event){
        switch (event->type)
        {
        case SDL_APP_TERMINATING:{
            /* Terminate the app.
               Shut everything down before returning from this function.
            */
            Base* base = BaseSingleton::getBase();
            base->shutdown();
            return 0;
        }
        case SDL_APP_LOWMEMORY:
            /* You will get this when your app is paused and iOS wants more memory.
               Release as much memory as possible.
            */
            return 0;
        case SDL_APP_WILLENTERBACKGROUND:
            /* Prepare your app to go into the background.  Stop loops, etc.
               This gets called when the user hits the home button, or gets a call.
            */
            return 0;
        case SDL_APP_DIDENTERBACKGROUND:
            /* This will get called if the user accepted whatever sent your app to the background.
               If the user got a phone call and canceled it, you'll instead get an    SDL_APP_DIDENTERFOREGROUND event and restart your loops.
               When you get this, you have 5 seconds to save all your state or the app will be terminated.
               Your app is NOT active at this point.
            */
            return 0;
        case SDL_APP_WILLENTERFOREGROUND:
           /* This call happens when your app is coming back to the foreground.
               Restore all your state here.
           */
            return 0;
        case SDL_APP_DIDENTERFOREGROUND:
            /* Restart your loops here.
               Your app is interactive and getting CPU again.
            */
            return 0;
        default:
            /* No special processing, add it to the event queue */
            return 1;
        }
    }

    void SDL2Window::_pollForEvents(){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            _handleEvent(event);
        }
    }

    void SDL2Window::_handleEvent(SDL_Event &event){
        switch(event.type){
            case SDL_QUIT:
                //close();
                wantsToClose = true;
                break;
            case SDL_WINDOWEVENT:
                switch(event.window.event){
                    case SDL_WINDOWEVENT_RESIZED:
                        _resizeWindow(event);
                        break;
                    case SDL_WINDOWEVENT_MINIMIZED:
                    case SDL_WINDOWEVENT_MAXIMIZED:
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                    case SDL_WINDOWEVENT_SHOWN:
                    case SDL_WINDOWEVENT_HIDDEN:
                    case SDL_WINDOWEVENT_RESTORED:
                    case SDL_WINDOWEVENT_EXPOSED:
                    {
                        _handleBasicWindowEvent(event.window);
                        break;
                    }
                }
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if(event.key.repeat == 0)
                    _handleKey(event.key.keysym, event.type == SDL_KEYDOWN);
                break;

            case SDL_TEXTINPUT:{
                mGuiInputProcessor->processTextInput(event.text.text);
                break;
            }
            case SDL_TEXTEDITING:{
                mGuiInputProcessor->processTextEdit(event.edit.text, event.edit.start, event.edit.length);
                break;
            }
            case SDL_MOUSEMOTION:{
                _handleMouseMotion(event.motion.x, event.motion.y);
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                _handleMouseButton((int)event.button.button, event.type == SDL_MOUSEBUTTONDOWN ? true : false);
                break;
            case SDL_MOUSEWHEEL:
                mInputManager->setMouseWheel(event.wheel.y);
                mGuiInputProcessor->processMouseScroll(0, event.wheel.y*2);
                break;
            case SDL_CONTROLLERAXISMOTION:{
                _handleControllerAxis(event);
                break;
            }
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:{
                _handleControllerButton(event);
                break;
            }
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:{
                _handleDeviceChange(event);
                break;
            }
            case SDL_JOYDEVICEADDED:{
                _handleJoystickAddition(event);
                break;
            }
            case SDL_FINGERDOWN:{
                mInputManager->notifyTouchBegan(event.tfinger.fingerId, event.tfinger.x, event.tfinger.y);
                bool guiIntersected = mInputManager->getMouseGuiIntersected();

                SystemEventInputTouchBegan e;
                e.fingerId = event.tfinger.fingerId;
                e.guiIntersected = guiIntersected;
                EventDispatcher::transmitEvent(EventType::System, e);
                break;
            }
            case SDL_FINGERUP:{
                SystemEventInputTouchEnded e;
                e.fingerId = event.tfinger.fingerId;
                EventDispatcher::transmitEvent(EventType::System, e);

                //Remove it after Squirrel has had a time to process the removal.
                mInputManager->notifyTouchEnded(event.tfinger.fingerId);
                break;
            }
            case SDL_FINGERMOTION:{
                mInputManager->notifyTouchMotion(event.tfinger.fingerId, event.tfinger.x, event.tfinger.y);

                SystemEventInputTouchMotion e;
                e.fingerId = event.tfinger.fingerId;
                EventDispatcher::transmitEvent(EventType::System, e);
                break;
            }
        }
    }

    void SDL2Window::injectOgreWindow(Ogre::Window *window){
        _ogreWindow = window;
        window->requestResolution(_width, _height);
    }

    Ogre::String SDL2Window::getX11Handle(SDL_SysWMinfo* wmInfo){
        //TODO Fix up the lifetime issues here.
        //I have to make it so the object does not get destroyed, which is difficult in the current design.
        SDL_VERSION( &(wmInfo->version) );

        if(!SDL_GetWindowWMInfo(_SDLWindow, wmInfo)){
            AV_CRITICAL("SDL failed to query window information to obtain the window handle: {}", SDL_GetError());
            return "0";
        }

        #if (defined(__linux__) || defined(__FreeBSD__)) && !defined(TARGET_ANDROID)
            return Ogre::StringConverter::toString((uintptr_t)&(wmInfo->info.x11));
        #else
            return "";
        #endif
    }

    void SDL2Window::extraOgreSetup(){
        #ifdef TARGET_APPLE_IPHONE
        SDL_SysWMinfo wmInfo;
        SDL_VERSION( &wmInfo.version );

        if(!SDL_GetWindowWMInfo(_SDLWindow, &wmInfo)){
            AV_CRITICAL("SDL failed to query window information to obtain the window handle: {}", SDL_GetError());
            assert(false);
        }

        AssignViewToSDLWindow(wmInfo, _ogreWindow);
        #endif
    }

    Window::ScreenSafeInsets SDL2Window::getScreenSafeAreaInsets() const{
        Window::ScreenSafeInsets insets;
        memset(&insets, 0, sizeof(Window::ScreenSafeInsets));

        #ifdef TARGET_APPLE_IPHONE
        SDL_SysWMinfo wmInfo;
        SDL_VERSION( &wmInfo.version );

        if(!SDL_GetWindowWMInfo(_SDLWindow, &wmInfo)){
            AV_CRITICAL("SDL failed to query window information to obtain the window handle: {}", SDL_GetError());
            assert(false);
        }

        NativeScreenSafeInsets screenInsets = GetScreenSafeAreaInsets(wmInfo);
        insets.left = screenInsets.left;
        insets.right = screenInsets.right;
        insets.top = screenInsets.top;
        insets.bottom = screenInsets.bottom;
        #endif

        return insets;
    }

    Ogre::String SDL2Window::getHandle(){
        SDL_SysWMinfo wmInfo;
        SDL_VERSION( &wmInfo.version );

        if(!SDL_GetWindowWMInfo(_SDLWindow, &wmInfo)){
            AV_CRITICAL("SDL failed to query window information to obtain the window handle: {}", SDL_GetError());
            assert(false);
        }

        #ifdef __APPLE__
            return Ogre::StringConverter::toString(WindowContentViewHandle(wmInfo));
        #elif defined(__linux__) || defined(__FreeBSD__)
            #if defined(TARGET_ANDROID)
                return Ogre::StringConverter::toString( (uintptr_t) wmInfo.info.android.window);
            #else
                return Ogre::StringConverter::toString( (uintptr_t) wmInfo.info.x11.window);
            #endif
        #elif _WIN32
            return Ogre::StringConverter::toString((uintptr_t)wmInfo.info.win.window);
        #endif
    }

    void SDL2Window::warpMouseInWindow(int x, int y){
        SDL_WarpMouseInWindow(_SDLWindow, x, y);

        int w, h;
        SDL_GL_GetDrawableSize(_SDLWindow, &w, &h);
        float actualWidth = (float)w / (float)_width;
        float actualHeight = (float)h / (float)_height;

        mInputManager->setActualMouseX(x * actualWidth);
        mInputManager->setActualMouseY(y * actualHeight);

        mInputManager->setMouseX(x);
        mInputManager->setMouseY(y);
    }

    int SDL2Window::getActualWidth() const{
        int w, h;
#ifdef TARGET_APPLE_IPHONE
        SDL_Metal_GetDrawableSize(_SDLWindow, &w, &h);
#else
        SDL_GL_GetDrawableSize(_SDLWindow, &w, &h);
#endif
        float actualWidth = (float)w / (float)_width;

        return _width * actualWidth;
    }

    int SDL2Window::getActualHeight() const{
        int w, h;
#ifdef TARGET_APPLE_IPHONE
        SDL_Metal_GetDrawableSize(_SDLWindow, &w, &h);
#else
        SDL_GL_GetDrawableSize(_SDLWindow, &w, &h);
#endif
        float actualHeight = (float)h / (float)_height;

        return _height * actualHeight;
    }

    void SDL2Window::grabCursor(bool capture){
        SDL_SetRelativeMouseMode(capture ? SDL_TRUE : SDL_FALSE);

        //int type = capture ? SDL_DISABLE : SDL_ENABLE;
        //SDL_ShowCursor(type);
    }

    void SDL2Window::showCursor(bool show){
        SDL_ShowCursor(show ? SDL_TRUE : SDL_FALSE);
    }

    void SDL2Window::setTitle(const std::string& title){
        Window::setTitle(title);
        SDL_SetWindowTitle(_SDLWindow, _currentTitle.c_str());
    }

    bool SDL2Window::showMessageBox(const MessageBoxData& msgData, int* pressedButton){
        SDL_MessageBoxData data;
        data.message = msgData.message.c_str();
        data.title = msgData.title.c_str();
        data.numbuttons = static_cast<int>(msgData.buttons.size());
        data.window = _SDLWindow;

        //Convert abstracted flags to sdl2 flags.
        uint32 sdl2Flags = 0;
        if(msgData.flags & MESSAGEBOX_ERROR) sdl2Flags |= SDL_MESSAGEBOX_ERROR;
        if(msgData.flags & MESSAGEBOX_WARNING) sdl2Flags |= SDL_MESSAGEBOX_WARNING;
        if(msgData.flags & MESSAGEBOX_INFORMATION) sdl2Flags |= SDL_MESSAGEBOX_INFORMATION;
        if(msgData.flags & MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT) sdl2Flags |= SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT;
        if(msgData.flags & MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT) sdl2Flags |= SDL_MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT;
        data.flags = sdl2Flags;

        SDL_MessageBoxButtonData* currentButton = static_cast<SDL_MessageBoxButtonData*>(malloc(sizeof(SDL_MessageBoxButtonData) * data.numbuttons));
        data.buttons = currentButton;

        for(int i = 0; i < data.numbuttons; i++){
            currentButton->text = msgData.buttons[i].c_str();
            currentButton->flags = 0;
            currentButton->buttonid = i;

            currentButton++;
        }

        int result = 0;
        if(msgData.buttons.empty()){
            result = SDL_ShowSimpleMessageBox(sdl2Flags, data.title, data.message, _SDLWindow);
        }else{
            result = SDL_ShowMessageBox(&data, pressedButton);
        }


        delete data.buttons;
        return result == 0;
    }

    void SDL2Window::_resizeWindow(SDL_Event &event){
        if(event.window.event != SDL_WINDOWEVENT_RESIZED) return;

        _setSize(event.window.data1, event.window.data2);
    }

    void SDL2Window::_handleControllerAxis(const SDL_Event& e){
        assert(e.type == SDL_CONTROLLERAXISMOTION);

        bool isTrigger = e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || e.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT;

        InputDeviceId deviceId = mRegisteredDevices[e.caxis.which];
        ActionHandle handle = inputMapper.getAxisMap(deviceId, (int)e.caxis.axis);
        //32767 is the maximum number sdl will return for an axis.
        float normValue = float(e.caxis.value) / 32767.0f;
        if(isTrigger){
            mInputManager->setAnalogTriggerAction(deviceId, handle, normValue);
        }else{
            //It's an actual axis.
            bool x = (e.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX || e.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) ? true : false;
            bool hitDeadzone = mInputManager->setAxisAction(deviceId, handle, x, normValue);
            if(!hitDeadzone){
                mGuiInputProcessor->processControllerAxis(inputMapper, (int)e.caxis.axis, normValue, x);
            }
        }
    }

    void SDL2Window::_handleControllerButton(const SDL_Event& e){
        assert(e.type == SDL_CONTROLLERBUTTONDOWN || e.type == SDL_CONTROLLERBUTTONUP);

        bool pressed = e.cbutton.state == SDL_PRESSED ? true : false;
        mGuiInputProcessor->processControllerButton(inputMapper, pressed, (int)e.cbutton.button);

        InputDeviceId deviceId = mRegisteredDevices[e.cbutton.which];
        ActionHandle handle = inputMapper.getButtonMap(deviceId, (int)e.cbutton.button);
        mInputManager->setButtonAction(deviceId, handle, pressed);
    }

    void SDL2Window::_addController(InputDeviceId i){
        //Only game controllers should appear in this function
        assert(SDL_IsGameController(i));

        const char* devName = SDL_GameControllerNameForIndex(i);
        if(!devName) devName = "Unknown device";
        InputDeviceId id = mInputManager->addInputDevice(devName);
        //If we've run out of available controllers, just don't add this new one.
        if(id == INVALID_INPUT_DEVICE) return;

        SDL_GameController* addDevice = SDL_GameControllerOpen(i);
        assert(addDevice);

        SDL_Joystick* j = SDL_GameControllerGetJoystick(addDevice);
        SDL_JoystickID joyId = SDL_JoystickInstanceID(j);

        //Here the joystick callback should have been called first. So we expect there to be some data in the list.
        if (mRegisteredDevices.size() > joyId) {
            //If there's enough space. It seems like there's no guarantee of whether the joystick add or the controller add function gets called first.
            //So I have to check whether the list needs adding to or not.
            mRegisteredDevices[joyId] = id;
        }else{
            mRegisteredDevices.push_back(id);
        }

        InputDeviceId actualId = (InputDeviceId)joyId;
        mOpenGameControllers[id] = {addDevice, actualId};
    }

    void SDL2Window::_handleDeviceChange(const SDL_Event& e){
        assert(e.type == SDL_CONTROLLERDEVICEADDED || e.type == SDL_CONTROLLERDEVICEREMOVED);

        InputDeviceId devId = e.cdevice.which;

        if(e.type == SDL_CONTROLLERDEVICEADDED){
            _addController(devId);
        }else{
            SDL_GameController* controller = SDL_GameControllerFromInstanceID(devId);
            assert(controller);

            int targetIdx = -1;
            //Find the id based on the pointer.
            for(int i = 0; i < MAX_INPUT_DEVICES; i++){
                if(controller == mOpenGameControllers[i].controller){
                    targetIdx = i;
                    break;
                }
            }
            assert(targetIdx >= 0); //It should be somewhere in the list.

            //mInputManager->removeInputDevice(mOpenGameControllers[targetIdx].controllerId);
            mInputManager->removeInputDevice(targetIdx);

            SDL_GameControllerClose(controller);
            mOpenGameControllers[targetIdx] = {0, 0};
        }
    }

    void SDL2Window::_handleJoystickAddition(const SDL_Event& e){
        //Process the addition or removal of an sdl joystick.
        //SDL game controllers really just sit ontop of the joystick api.
        //The engine's SDL implementation does not support joysticks, however their use can interfere with the id system of actual game controllers.
        //This can happen if the os has bad driver support and a controller gets sent as a joystick (or the user actually connects a joystick, not that anyone ownes them anymore).
        //So to avoid the id system getting messed up I need to maintain my own mapping of these sdl ids to actual engine device ids.
        //This is done with a vector which grows as devices are added.
        //It allows an O(1) lookup of the actual id.
        //The vector is never removed from however, as that would mess up the direct lookup.
        //This does mean an ever growing list, however, I've decided this is acceptable for the following reasons:
        //1.Devices are hardly ever added. If a memory overflow was to occur the user would have to be trying very hard.
        //2.It stores entries of eight bits, so pretty small.
        //3. SDL is much more likely to give up before this list grows big enough that it's a problem.
        //I could use a map but they're slower (this lookup is going to happen each time a button is pressed for instance).

        if(SDL_IsGameController(e.jdevice.which)) return;

        //assert(mRegisteredDevices.size() == e.jdevice.which);
        /*if(mRegisteredDevices.size() <= e.jdevice.which){
            mRegisteredDevices.push_back(INVALID_INPUT_DEVICE); //Joysticks aren't supported, so it should be mapped as an invalid device.
        }else{
            mRegisteredDevices[e.jdevice.which] = INVALID_INPUT_DEVICE;
        }*/
        mRegisteredDevices.push_back(INVALID_INPUT_DEVICE); //Joysticks aren't supported, so it should be mapped as an invalid device.
        //Each time a joystick is registered the list should grow.
    }

    void SDL2Window::_handleKey(SDL_Keysym key, bool pressed){
        if(key.scancode == SDL_SCANCODE_UNKNOWN || key.scancode == SDL_SCANCODE_LGUI) return;

        if(pressed && key.scancode == SDL_SCANCODE_F1){
            DebuggerToolEventToggle event;
            event.t = DebuggerToolToggle::StatsToggle;
            EventDispatcher::transmitEvent(EventType::DebuggerTools, event);
        }
        else if(pressed && key.scancode == SDL_SCANCODE_F2){
            DebuggerToolEventToggle event;
            event.t = DebuggerToolToggle::MeshesToggle;
            EventDispatcher::transmitEvent(EventType::DebuggerTools, event);
        }
        int keyCode = (int)(key.sym);
        mGuiInputProcessor->processInputKey(inputMapper, pressed, (int)key.scancode, keyCode, (int)key.mod, isKeyboardInputEnabled);

        ActionHandle handle = inputMapper.getKeyboardMap(keyCode);

        mInputManager->setKeyboardKeyAction(handle, pressed ? 1.0f : 0.0f);
        mInputManager->setKeyboardInput(key.scancode, pressed);
    }

    void SDL2Window::_handleMouseButton(int button, bool pressed){
        int targetButton = 0;
        switch(button){
            case SDL_BUTTON_LEFT:
                targetButton = 0;
                break;
            case SDL_BUTTON_RIGHT:
                targetButton = 1;
                break;
            case SDL_BUTTON_MIDDLE:
                targetButton = 2;
                break;
        }

        bool intersectedGui = mGuiInputProcessor->processMouseButton(targetButton, pressed);
        mInputManager->setMouseButton(targetButton, pressed, intersectedGui);
    }

    void SDL2Window::_handleMouseMotion(float x, float y){
        int w, h;
        SDL_GL_GetDrawableSize(_SDLWindow, &w, &h);
        float actualWidth = w / _width;
        float actualHeight = h / _height;

        mGuiInputProcessor->processMouseMove(x / _width, y / _height);

        mInputManager->setActualMouseX(x * actualWidth);
        mInputManager->setActualMouseY(y * actualHeight);

        mInputManager->setMouseX(x);
        mInputManager->setMouseY(y);

    }

    void SDL2Window::_setupSystemCursors(){
        static const SDL_SystemCursor cursor[NUM_SYSTEM_CURSORS] = {
            SDL_SYSTEM_CURSOR_ARROW,
            SDL_SYSTEM_CURSOR_IBEAM,
            SDL_SYSTEM_CURSOR_WAIT,
            SDL_SYSTEM_CURSOR_CROSSHAIR,
            SDL_SYSTEM_CURSOR_WAITARROW,
            SDL_SYSTEM_CURSOR_SIZENWSE,
            SDL_SYSTEM_CURSOR_SIZENESW,
            SDL_SYSTEM_CURSOR_SIZEWE,
            SDL_SYSTEM_CURSOR_SIZENS,
            SDL_SYSTEM_CURSOR_SIZEALL,
            SDL_SYSTEM_CURSOR_NO,
            SDL_SYSTEM_CURSOR_HAND,
        };
        for(int i = 0; i < 8; i++){
            mSystemCursors[i] = SDL_CreateSystemCursor(cursor[i]);
            //printf(" %s\n", SDL_GetError());
        }
    }

    void SDL2Window::_destroySystemCursors(){
        for(int i = 0; i < 8; i++){
            SDL_FreeCursor(mSystemCursors[i]);
        }
    }

    bool SDL2Window::isInitialised(){
        return _initialised;
    }

    bool SDL2Window::isOpen(){
        return _open;
    }

    void SDL2Window::setSystemCursor(SystemCursor cursor){
        size_t idx = static_cast<size_t>(cursor);
        SDL_SetCursor(mSystemCursors[idx]);
    }

    void SDL2Window::setPosition(int x, int y){
        SDL_SetWindowPosition(_SDLWindow, x, y);
    }

    int SDL2Window::getPositionX() const{
        int x, y;
        SDL_GetWindowPosition(_SDLWindow, &x, &y);
        return x;
    }

    int SDL2Window::getPositionY() const{
        int x, y;
        SDL_GetWindowPosition(_SDLWindow, &x, &y);
        return y;
    }

    void SDL2Window::rumbleInputDevice(InputDeviceId device, float lowFreqStrength, float highFreqStrength, uint32 rumbleTimeMs){
        assert(device < MAX_INPUT_DEVICES);
        const ControllerEntry& e = mOpenGameControllers[device];

        uint16 low = static_cast<uint16>(lowFreqStrength * 0xFFFF);
        uint16 high = static_cast<uint16>(highFreqStrength * 0xFFFF);
        SDL_GameControllerRumble(e.controller, low, high, rumbleTimeMs);
    }

}
