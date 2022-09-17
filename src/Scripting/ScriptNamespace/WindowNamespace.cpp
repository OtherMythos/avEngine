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
    }
}
