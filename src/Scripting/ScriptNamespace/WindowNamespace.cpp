#include "WindowNamespace.h"

#include "System/BaseSingleton.h"
#include "Window/Window.h"
#include "Window/SDL2Window/SDL2Window.h"

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

    SQInteger WindowNamespace::grabCursor(HSQUIRRELVM vm){
        SQBool enable;
        sq_getbool(vm, -1, &enable);

        SDL2Window* sdlWindow = static_cast<SDL2Window*>(BaseSingleton::getWindow());

        sdlWindow->grabCursor(enable);

        return 0;
    }

    SQInteger WindowNamespace::getRenderTexture(HSQUIRRELVM vm){
        SDL2Window* sdlWindow = static_cast<SDL2Window*>(BaseSingleton::getWindow());
        Ogre::TextureGpu* texture = sdlWindow->getRenderWindow()->getTexture();

        TextureUserData::textureToUserData(vm, texture, false);

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
        @name grabCursor
        @desc Set the cursor to be grabbed to the window. If this is enabled it will be made invisible and unable to exit the window. This is useful for FPS style programs.
        @param1:grab: A boolean value of whether the cursor should be grabbed or not.
        */
        ScriptUtils::addFunction(vm, grabCursor, "grabCursor", 2, ".b");

        ScriptUtils::addFunction(vm, getRenderTexture, "getRenderTexture");
    }
}
