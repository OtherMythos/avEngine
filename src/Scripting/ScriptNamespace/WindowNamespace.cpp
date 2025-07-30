#include "WindowNamespace.h"

#include "System/BaseSingleton.h"
#include "Window/Window.h"
#include "Window/SDL2Window/SDL2Window.h"
#include "Input/InputManager.h"

#include "OgreWindow.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/TextureUserData.h"
#include "Scripting/ScriptNamespace/Classes/Vector2UserData.h"

#ifdef __APPLE__
    #include "Window/SDL2Window/MacOS/MacOSUtils.h"
#endif

namespace AV{

    SQInteger WindowNamespace::getSize(HSQUIRRELVM vm){
        Window* win = BaseSingleton::getWindow();
        Ogre::Vector2 winSize(win->getWidth(), win->getHeight());
        Vector2UserData::vector2ToUserData(vm, winSize);

        return 1;
    }

    SQInteger WindowNamespace::getActualSize(HSQUIRRELVM vm){
        Window* win = BaseSingleton::getWindow();
        Ogre::Vector2 winSize(win->getActualWidth(), win->getActualHeight());
        Vector2UserData::vector2ToUserData(vm, winSize);

        return 1;
    }

    SQInteger WindowNamespace::getWidth(HSQUIRRELVM vm){
        sq_pushinteger(vm, BaseSingleton::getWindow()->getWidth());

        return 1;
    }

    SQInteger WindowNamespace::getHeight(HSQUIRRELVM vm){
        sq_pushinteger(vm, BaseSingleton::getWindow()->getHeight());

        return 1;
    }

    SQInteger WindowNamespace::getActualWidth(HSQUIRRELVM vm){
        sq_pushinteger(vm, BaseSingleton::getWindow()->getActualWidth());

        return 1;
    }

    SQInteger WindowNamespace::getScreenSafeAreaInsets(HSQUIRRELVM vm){
        Window::ScreenSafeInsets insets = BaseSingleton::getWindow()->getScreenSafeAreaInsets();

        sq_newtable(vm);

        sq_pushstring(vm, "top", -1);
        sq_pushfloat(vm, insets.top);
        sq_newslot(vm, -3, SQFalse);

        sq_pushstring(vm, "bottom", -1);
        sq_pushfloat(vm, insets.bottom);
        sq_newslot(vm, -3, SQFalse);

        sq_pushstring(vm, "left", -1);
        sq_pushfloat(vm, insets.left);
        sq_newslot(vm, -3, SQFalse);

        sq_pushstring(vm, "right", -1);
        sq_pushfloat(vm, insets.right);
        sq_newslot(vm, -3, SQFalse);

        return 1;
    }

    SQInteger WindowNamespace::getActualHeight(HSQUIRRELVM vm){
        sq_pushinteger(vm, BaseSingleton::getWindow()->getActualHeight());

        return 1;
    }

    SQInteger WindowNamespace::grabCursor(HSQUIRRELVM vm){
        SQBool enable;
        sq_getbool(vm, -1, &enable);

        SDL2Window* sdlWindow = static_cast<SDL2Window*>(BaseSingleton::getWindow());

        sdlWindow->grabCursor(enable);

        return 0;
    }

    SQInteger WindowNamespace::showCursor(HSQUIRRELVM vm){
        SQBool enable;
        sq_getbool(vm, -1, &enable);

        SDL2Window* sdlWindow = static_cast<SDL2Window*>(BaseSingleton::getWindow());

        sdlWindow->showCursor(enable);

        return 0;
    }

    SQInteger WindowNamespace::setCursor(HSQUIRRELVM vm){
        SQInteger cursorId;
        sq_getinteger(vm, -1, &cursorId);

        if(cursorId < 0 || cursorId >= (int)Window::SystemCursor::CURSOR_MAX) return 0;

        Window* window = BaseSingleton::getWindow();

        Window::SystemCursor sysCursor = static_cast<Window::SystemCursor>(cursorId);
        window->setSystemCursor(sysCursor);

        return 0;
    }

    SQInteger WindowNamespace::warpMouseInWindow(HSQUIRRELVM vm){
        SQInteger x, y;
        sq_getinteger(vm, 2, &x);
        sq_getinteger(vm, 3, &y);

        //TODO move some of these functions to the base class and dynamic cast to typecheck.
        SDL2Window* sdlWindow = static_cast<SDL2Window*>(BaseSingleton::getWindow());

        sdlWindow->warpMouseInWindow(static_cast<int>(x), static_cast<int>(y));

        return 0;
    }

    SQInteger WindowNamespace::getRenderTexture(HSQUIRRELVM vm){
        SDL2Window* sdlWindow = static_cast<SDL2Window*>(BaseSingleton::getWindow());
        Ogre::TextureGpu* texture = sdlWindow->getRenderWindow()->getTexture();

        TextureUserData::textureToUserData(vm, texture, false);

        return 1;
    }

    SQInteger _getFullscreenMode(HSQUIRRELVM vm, FullscreenMode* outMode){
        FullscreenMode targetMode = FullscreenMode::WINDOWED;

        SQObjectType foundType = sq_gettype(vm, -1);
        if(foundType == OT_BOOL){
            SQBool enable;
            sq_getbool(vm, -1, &enable);

            targetMode = enable ? FullscreenMode::FULLSCREEN : FullscreenMode::WINDOWED;
        }else if(foundType == OT_INTEGER){
            SQInteger value;
            sq_getinteger(vm, -1, &value);
            targetMode = static_cast<FullscreenMode>(value);

            if(
                targetMode != FullscreenMode::WINDOWED &&
                targetMode != FullscreenMode::FULLSCREEN &&
                targetMode != FullscreenMode::FULLSCREEN_BORDERLESS
            ){
                return sq_throwerror(vm, "Invalid fullscreen mode provided.");
            }
        }

        *outMode = targetMode;
        return 0;
    }
    SQInteger WindowNamespace::setFullscreen(HSQUIRRELVM vm){
        FullscreenMode targetMode = FullscreenMode::WINDOWED;
        SQInteger result = _getFullscreenMode(vm, &targetMode);
        if(SQ_FAILED(result)){
            return result;
        }

        bool fullscreenResult = BaseSingleton::getWindow()->setFullscreen(targetMode);
        if(!fullscreenResult){
            return sq_throwerror(vm, "Error setting window fullscreen value.");
        }

        return 0;
    }

    SQInteger WindowNamespace::setupFuncSetFullscreen(HSQUIRRELVM vm){
        FullscreenMode targetMode = FullscreenMode::WINDOWED;
        SQInteger result = _getFullscreenMode(vm, &targetMode);
        if(SQ_FAILED(result)){
            return result;
        }

        SystemSettings::setDefaultFullscreenMode(targetMode);

        return 0;
    }

    SQInteger WindowNamespace::getFullscreen(HSQUIRRELVM vm){
        SDL2Window* sdlWindow = static_cast<SDL2Window*>(BaseSingleton::getWindow());

        bool result = sdlWindow->getFullscreen();
        sq_pushbool(vm, result);

        return 1;
    }

    SQInteger WindowNamespace::setBorderless(HSQUIRRELVM vm) {
        SDL2Window* sdlWindow = static_cast<SDL2Window*>(BaseSingleton::getWindow());

        SQBool enable;
        sq_getbool(vm, -1, &enable);

        sdlWindow->setBorderless(enable);

        return 0;
    }

    SQInteger WindowNamespace::getBorderless(HSQUIRRELVM vm) {
        SDL2Window* sdlWindow = static_cast<SDL2Window*>(BaseSingleton::getWindow());

        bool result = sdlWindow->getBorderless();
        sq_pushbool(vm, result);

        return 1;
    }

    SQInteger WindowNamespace::getTitle(HSQUIRRELVM vm){
        const std::string& windowTitle = BaseSingleton::getWindow()->getTitle();

        sq_pushstring(vm, windowTitle.c_str(), windowTitle.length());

        return 1;
    }

    SQInteger WindowNamespace::setTitle(HSQUIRRELVM vm){
        const SQChar *title;
        sq_getstring(vm, -1, &title);

        BaseSingleton::getWindow()->setTitle(title);

        return 0;
    }

    SQInteger WindowNamespace::getNumDisplays(HSQUIRRELVM vm){
        //TODO move this out of the namespace.
        int numDisplays = SDL_GetNumVideoDisplays();

        sq_pushinteger(vm, numDisplays);

        return 1;
    }

    SQInteger _showMessageBoxIterateButtonsArray(HSQUIRRELVM vm, std::vector<std::string>& buttons){
        sq_pushnull(vm);
        while(SQ_SUCCEEDED(sq_next(vm, -2))){

            SQObjectType t = sq_gettype(vm, -1);
            if(t == OT_STRING){
                const SQChar *k;
                sq_getstring(vm, -1, &k);

                buttons.push_back(k);
            }

            sq_pop(vm, 2);
        }

        sq_pop(vm, 1);

        return 0;
    }

    SQInteger WindowNamespace::showMessageBox(HSQUIRRELVM vm){
        Window::MessageBoxData boxData;
        boxData.buttons.clear();


        sq_pushnull(vm);
        while(SQ_SUCCEEDED(sq_next(vm,-2))){
            const SQChar *k;
            sq_getstring(vm, -2, &k);

            SQObjectType t = sq_gettype(vm, -1);
            if(t == OT_STRING){
                const SQChar *stringVal;
                sq_getstring(vm, -1, &stringVal);

                if(strcmp(k, "title") == 0){
                    boxData.title = stringVal;
                }
                else if(strcmp(k, "message") == 0){
                    boxData.message = stringVal;
                }
            }else if(t == OT_INTEGER){
                SQInteger intVal;
                sq_getinteger(vm, -1, &intVal);

                if(strcmp(k, "flags") == 0){
                    boxData.flags = static_cast<uint32>(intVal);
                }
            }else if(t == OT_ARRAY){
                if(strcmp(k, "buttons") == 0){
                    _showMessageBoxIterateButtonsArray(vm, boxData.buttons);
                }
            }

            sq_pop(vm,2); //pop the key and value
        }

        sq_pop(vm,1); //pops the null iterator


        int pressedButton = -1;
        bool success = BaseSingleton::getWindow()->showMessageBox(boxData, &pressedButton);

        if(!success){
            return sq_throwerror(vm, "System error when opening message box.");
        }

        sq_pushinteger(vm, pressedButton);
        return 1;
    }

    SQInteger WindowNamespace::getWindowX(HSQUIRRELVM vm){
        int xPos = BaseSingleton::getWindow()->getPositionX();
        sq_pushinteger(vm, xPos);

        return 1;
    }

    SQInteger WindowNamespace::getWindowY(HSQUIRRELVM vm){
        int yPos = BaseSingleton::getWindow()->getPositionY();
        sq_pushinteger(vm, yPos);

        return 1;
    }

    SQInteger WindowNamespace::setWindowPosition(HSQUIRRELVM vm){
        SQInteger x, y;
        sq_getinteger(vm, 2, &x);
        sq_getinteger(vm, 3, &y);

        BaseSingleton::getWindow()->setPosition(static_cast<int>(x), static_cast<int>(y));

        return 0;
    }

    SQInteger WindowNamespace::setSize(HSQUIRRELVM vm){
        SQInteger x, y;
        sq_getinteger(vm, 2, &x);
        sq_getinteger(vm, 3, &y);

        BaseSingleton::getWindow()->setSize(static_cast<int>(x), static_cast<int>(y));

        return 0;
    }

    SQInteger WindowNamespace::getWindowDisplayIndex(HSQUIRRELVM vm){
        int index = BaseSingleton::getWindow()->getWindowDisplayIndex();

        sq_pushinteger(vm, index);

        return 1;
    }

    SQInteger WindowNamespace::getDisplayPositionCoordinates(HSQUIRRELVM vm){
        SQInteger idx;
        sq_getinteger(vm, 2, &idx);

        SDL_Rect rect;
        if(SDL_GetDisplayBounds(static_cast<int>(idx), &rect) < 0){
            std::string error = "Unable to find display with idx ";
            error += std::to_string(idx);
            return sq_throwerror(vm, error.c_str());
        }

        Vector2UserData::vector2ToUserData(vm, Ogre::Vector2(rect.x, rect.y));

        return 1;
    }

    SQInteger WindowNamespace::getDisplaySize(HSQUIRRELVM vm){
        SQInteger idx;
        sq_getinteger(vm, 2, &idx);

        SDL_Rect rect;
        if(SDL_GetDisplayBounds(static_cast<int>(idx), &rect) < 0){
            std::string error = "Unable to find display with idx ";
            error += std::to_string(idx);
            return sq_throwerror(vm, error.c_str());
        }

        Vector2UserData::vector2ToUserData(vm, Ogre::Vector2(rect.w, rect.h));

        return 1;
    }

    SQInteger WindowNamespace::setupFuncGetIntendedWidth(HSQUIRRELVM vm){
        Ogre::uint32 windowWidth = SystemSettings::getDefaultWindowWidth();

        sq_pushinteger(vm, static_cast<SQInteger>(windowWidth));

        return 1;
    }

    SQInteger WindowNamespace::setupFuncGetIntendedHeight(HSQUIRRELVM vm){
        Ogre::uint32 windowHeight = SystemSettings::getDefaultWindowHeight();

        sq_pushinteger(vm, static_cast<SQInteger>(windowHeight));

        return 1;
    }

    SQInteger WindowNamespace::setupFuncSetIntendedWidth(HSQUIRRELVM vm){
        SQInteger width;
        sq_getinteger(vm, -1, &width);

        SystemSettings::setDefaultWidth(static_cast<Ogre::uint32>(width));

        return 0;
    }

    SQInteger WindowNamespace::setupFuncSetIntendedHeight(HSQUIRRELVM vm){
        SQInteger height;
        sq_getinteger(vm, -1, &height);

        SystemSettings::setDefaultHeight(static_cast<Ogre::uint32>(height));

        return 0;
    }

    void WindowNamespace::setupSetupFuncNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name getIntendedWidth
        @returns The intended width for the window, which is the width the window will be once setup has been finished.
        */
        ScriptUtils::addFunction(vm, setupFuncGetIntendedWidth, "getIntendedWidth");
        /**SQFunction
        @name getIntendedHeight
        @returns The intended height for the window, which is the height the window will be once setup has been finished.
        */
        ScriptUtils::addFunction(vm, setupFuncGetIntendedWidth, "getIntendedHeight");
        /**SQFunction
        @name setIntendedWidth
        @returns The intended width for the window, which is the width the window will be once setup has been finished.
        */
        ScriptUtils::addFunction(vm, setupFuncSetIntendedWidth, "setIntendedWidth", 2, ".i");
        /**SQFunction
        @name setIntendedHeight
        @returns The intended height for the window, which is the height the window will be once setup has been finished.
        */
        ScriptUtils::addFunction(vm, setupFuncSetIntendedHeight, "setIntendedHeight", 2, ".i");

        ScriptUtils::addFunction(vm, getDisplaySize, "getDisplaySize", 2, ".i");
        /**SQFunction
        @name setFullscreen
        @desc Set the fullscreen mode for when the window is created.
        @param1:integer: Flag to specify fullscreen. Either _WINDOW_WINDOWED, _WINDOW_FULLSCREEN or _WINDOW_FULLSCREEN_BORDERLESS.
        */
        ScriptUtils::addFunction(vm, setupFuncSetFullscreen, "setDefaultFullscreen", 2, ".i");

    }

    /**SQNamespace
    @name _window
    @desc A namespace to interact with the window system.
    */
    void WindowNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name getSize
        @returns The size of the window as a Vec2
        */
        ScriptUtils::addFunction(vm, getSize, "getSize");
        /**SQFunction
        @name getActualSize
        @returns The actual size of the window as a Vec2. This is used when supporting higher resolution displays.
        */
        ScriptUtils::addFunction(vm, getActualSize, "getActualSize");
        /**SQFunction
        @name getWidth
        @returns The width of the window as an int.
        */
        ScriptUtils::addFunction(vm, getWidth, "getWidth");
        /**SQFunction
        @name getHeight
        @returns The height of the window as an int.
        */
        ScriptUtils::addFunction(vm, getHeight, "getHeight");
        /**SQFunction
        @name getActualWidth
        @returns The actual width of the window as an int. This value should be used when supporting higher resolution displays.
        */
        ScriptUtils::addFunction(vm, getActualWidth, "getActualWidth");
        /**SQFunction
        @name getActualHeight
        @returns The actual height of the window as an int. This value should be used when supporting higher resolution displays.
        */
        ScriptUtils::addFunction(vm, getActualHeight, "getActualHeight");
        /**SQFunction
        @name getNumDisplays
        @returns The number of displays currently available.
        */
        ScriptUtils::addFunction(vm, getNumDisplays, "getNumDisplays");
        /**SQFunction
        @name getPositionX
        @returns The X coordinate of the window.
        */
        ScriptUtils::addFunction(vm, getWindowX, "getPositionX");
        /**SQFunction
        @name getPositionY
        @returns The Y coordinate of the window.
        */
        ScriptUtils::addFunction(vm, getWindowY, "getPositionY");

        /**SQFunction
        @name grabCursor
        @desc Set the cursor to be grabbed to the window. If this is enabled it will be made invisible and unable to exit the window. This is useful for FPS style programs.
        @param1:grab: A boolean value of whether the cursor should be grabbed or not.
        */
        ScriptUtils::addFunction(vm, grabCursor, "grabCursor", 2, ".b");
        /**SQFunction
        @name showCursor
        @desc Set the window cursor to either visible or invisible.
        @param1:boolean: A boolean value of whether the cursor should be visible or not.
        */
        ScriptUtils::addFunction(vm, showCursor, "showCursor", 2, ".b");
        /**SQFunction
        @name setCursor
        @desc Set the current cursor.
        @param1:integer: Constant id of the system cursor to use.
        */
        ScriptUtils::addFunction(vm, setCursor, "setCursor", 2, ".i");
        /**SQFunction
        @name getRenderTexture
        @desc Get the render texture for the window. Can be used to setup compositors.
        @returns A texture object representing the window render target.
        */
        ScriptUtils::addFunction(vm, getRenderTexture, "getRenderTexture");
        /**SQFunction
        @name warpMouseInWindow
        @desc Warp the mouse to the specified position in the window.
        @param1:integer: X coordinate in the window.
        @param2:integer: Y coordinate in the window.
        */
        ScriptUtils::addFunction(vm, warpMouseInWindow, "warpMouseInWindow", 3, ".ii");
        /**SQFunction
        @name setFullscreen
        @desc Set the window to be fullscreen.
        @param1:boolean: Whether the window should be fullscreen.
        @param1:integer: Flag to specify fullscreen. Either _WINDOW_WINDOWED, _WINDOW_FULLSCREEN or _WINDOW_FULLSCREEN_BORDERLESS.
        */
        ScriptUtils::addFunction(vm, setFullscreen, "setFullscreen", 2, ".b|i");
        /**SQFunction
        @name getFullscreen
        @desc Get the fullscreen status of the window.
        @returns True or false depending on whether the window is fullscreen.
        */
        ScriptUtils::addFunction(vm, getFullscreen, "getFullscreen");
        /**SQFunction
        @name setBorderless
        @desc Set the window to be borderless.
        @param1:boolean: Whether the window should be borderless.
        */
        ScriptUtils::addFunction(vm, setBorderless, "setBorderless", 2, ".b");
        /**SQFunction
        @name getBorderless
        @desc Get the borderless state of the window.
        @returns True or false depending on whether the window is borderless.
        */
        ScriptUtils::addFunction(vm, getBorderless, "getBorderless");
        /**SQFunction
        @name getTitle
        @desc Get the current title of the window.
        @returns The window title as a string.
        */
        ScriptUtils::addFunction(vm, getTitle, "getTitle");
        /**SQFunction
        @name getWindowDisplayIndex
        @desc Get the index of the display corresponding to the one on which the window is currently.
        @returns The display index as an integer.
        */
        ScriptUtils::addFunction(vm, getWindowDisplayIndex, "getWindowDisplayIndex");
        /**SQFunction
        @name getDisplayPositionCoordinates
        @desc Get the x and y position of the provided display.
        @returns Vector2 representing the position.
        */
        ScriptUtils::addFunction(vm, getDisplayPositionCoordinates, "getDisplayPositionCoordinates", 2, ".i");
        /**SQFunction
        @name getDisplaySize
        @desc Get the width and height of the provided display.
        @returns Vector2 representing the position.
        */
        ScriptUtils::addFunction(vm, getDisplaySize, "getDisplaySize", 2, ".i");
        /**SQFunction
        @name setTitle
        @desc Set the current title of the window.
        @param1:string: The new title of the window.
        */
        ScriptUtils::addFunction(vm, setTitle, "setTitle");
        /**SQFunction
        @name showMessageBox
        @desc Show a modal message box.
        @param1:table: Table containing data for the message box. //TODO more info please.
        */
        ScriptUtils::addFunction(vm, showMessageBox, "showMessageBox");
        /**SQFunction
        @name setPosition
        @desc Set the position of the window.
        @param1:integer: The x position of the window.
        @param2:integer: The y position of the window.
        */
        ScriptUtils::addFunction(vm, setWindowPosition, "setPosition", 3, ".ii");
        /**SQFunction
        @name setSize
        @desc Set the size of the window.
        @param1:integer: The width of the window.
        @param2:integer: The height of the window.
        */
        ScriptUtils::addFunction(vm, setSize, "setSize", 3, ".ii");
        /**SQFunction
        @name getScreenSafeAreaInsets
        @desc Get the safe area for the screen. For instance on mobile if a camera notch is present on the device the size of the notch will be included.
        */
        ScriptUtils::addFunction(vm, getScreenSafeAreaInsets, "getScreenSafeAreaInsets");
    }

    void WindowNamespace::setupConstants(HSQUIRRELVM vm){
        ScriptUtils::declareConstant(vm, "_MESSAGEBOX_ERROR", (SQInteger)Window::MESSAGEBOX_ERROR);
        ScriptUtils::declareConstant(vm, "_MESSAGEBOX_WARNING", (SQInteger)Window::MESSAGEBOX_WARNING);
        ScriptUtils::declareConstant(vm, "_MESSAGEBOX_INFORMATION", (SQInteger)Window::MESSAGEBOX_INFORMATION);
        ScriptUtils::declareConstant(vm, "_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT", (SQInteger)Window::MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT);
        ScriptUtils::declareConstant(vm, "_MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT", (SQInteger)Window::MESSAGEBOX_BUTTONS_RIGHT_TO_LEFT);

        ScriptUtils::declareConstant(vm, "_SYSTEM_CURSOR_ARROW", (SQInteger)Window::SystemCursor::CURSOR_ARROW);
        ScriptUtils::declareConstant(vm, "_SYSTEM_CURSOR_IBEAM", (SQInteger)Window::SystemCursor::CURSOR_IBEAM);
        ScriptUtils::declareConstant(vm, "_SYSTEM_CURSOR_WAIT", (SQInteger)Window::SystemCursor::CURSOR_WAIT);
        ScriptUtils::declareConstant(vm, "_SYSTEM_CURSOR_CROSSHAIR", (SQInteger)Window::SystemCursor::CURSOR_CROSSHAIR);
        ScriptUtils::declareConstant(vm, "_SYSTEM_CURSOR_WAITARROW", (SQInteger)Window::SystemCursor::CURSOR_WAITARROW);
        ScriptUtils::declareConstant(vm, "_SYSTEM_CURSOR_SIZENWSE", (SQInteger)Window::SystemCursor::CURSOR_SIZENWSE);
        ScriptUtils::declareConstant(vm, "_SYSTEM_CURSOR_SIZENESW", (SQInteger)Window::SystemCursor::CURSOR_SIZENESW);
        ScriptUtils::declareConstant(vm, "_SYSTEM_CURSOR_SIZEWE", (SQInteger)Window::SystemCursor::CURSOR_SIZEWE);
        ScriptUtils::declareConstant(vm, "_SYSTEM_CURSOR_SIZENS", (SQInteger)Window::SystemCursor::CURSOR_SIZENS);
        ScriptUtils::declareConstant(vm, "_SYSTEM_CURSOR_SIZEALL", (SQInteger)Window::SystemCursor::CURSOR_SIZEALL);
        ScriptUtils::declareConstant(vm, "_SYSTEM_CURSOR_NO", (SQInteger)Window::SystemCursor::CURSOR_NO);
        ScriptUtils::declareConstant(vm, "_SYSTEM_CURSOR_HAND", (SQInteger)Window::SystemCursor::CURSOR_HAND);

        ScriptUtils::declareConstant(vm, "_WINDOW_WINDOWED", (SQInteger)FullscreenMode::WINDOWED);
        ScriptUtils::declareConstant(vm, "_WINDOW_FULLSCREEN", (SQInteger)FullscreenMode::FULLSCREEN);
        ScriptUtils::declareConstant(vm, "_WINDOW_FULLSCREEN_BORDERLESS", (SQInteger)FullscreenMode::FULLSCREEN_BORDERLESS);
    }
}
