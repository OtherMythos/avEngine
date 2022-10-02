#include "WindowNamespace.h"

#include "System/BaseSingleton.h"
#include "Window/Window.h"
#include "Window/SDL2Window/SDL2Window.h"
#include "Input/InputManager.h"

#include "OgreWindow.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/TextureUserData.h"

namespace AV{

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

        sdlWindow->warpMouseInWindow(x, y);

        return 0;
    }

    SQInteger WindowNamespace::getRenderTexture(HSQUIRRELVM vm){
        SDL2Window* sdlWindow = static_cast<SDL2Window*>(BaseSingleton::getWindow());
        Ogre::TextureGpu* texture = sdlWindow->getRenderWindow()->getTexture();

        TextureUserData::textureToUserData(vm, texture, false);

        return 1;
    }

    SQInteger WindowNamespace::setFullscreen(HSQUIRRELVM vm){
        SQBool enable;
        sq_getbool(vm, -1, &enable);

        //SDL2Window* sdlWindow = static_cast<SDL2Window*>(BaseSingleton::getWindow());

        bool result = BaseSingleton::getWindow()->setFullscreen(enable);
        if(!result){
            return sq_throwerror(vm, "Error setting window to fullscreen.");
        }

        return 1;
    }

    SQInteger WindowNamespace::getFullscreen(HSQUIRRELVM vm){
        SDL2Window* sdlWindow = static_cast<SDL2Window*>(BaseSingleton::getWindow());

        bool result = sdlWindow->getFullscreen();
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

    /**SQNamespace
    @name _window
    @desc A namespace to interact with the window system.
    */
    void WindowNamespace::setupNamespace(HSQUIRRELVM vm){
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
        */
        ScriptUtils::addFunction(vm, setFullscreen, "setFullscreen", 2, ".b");
        /**SQFunction
        @name getFullscreen
        @desc Get the fullscreen status of the window.
        @returns True or false depending on whether the window is fullscreen.
        */
        ScriptUtils::addFunction(vm, getFullscreen, "getFullscreen");
        /**SQFunction
        @name getTitle
        @desc Get the current title of the window.
        @returns The window title as a string.
        */
        ScriptUtils::addFunction(vm, getTitle, "getTitle");
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
    }
}
