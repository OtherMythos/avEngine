#pragma once

#include <squirrel.h>

#include "Scripting/ScriptDataPacker.h"
#include "Gui/Texture2d/MovableTextureManager.h"

namespace AV{

    /**
    A class to expose movable textures to squirrel.
    */
    class MovableTextureClass{
    public:
        MovableTextureClass() { };

        static void setupClass(HSQUIRRELVM vm);

    private:
        static ScriptDataPacker<MovableTexturePtr> mTextures;
        static SQObject classObject;

        static SQInteger sqTextureReleaseHook(SQUserPointer p, SQInteger size);

        static SQInteger movableTextureConstructor(HSQUIRRELVM vm);

    };
}
