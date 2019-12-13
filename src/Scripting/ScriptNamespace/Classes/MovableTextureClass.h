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

        static SQInteger setTexturePosition(HSQUIRRELVM vm);

        static SQInteger setTextureWidth(HSQUIRRELVM vm);
        static SQInteger setTextureHeight(HSQUIRRELVM vm);
        static SQInteger setTextureSize(HSQUIRRELVM vm);

        static SQInteger setTexture(HSQUIRRELVM vm);
        static SQInteger setLayer(HSQUIRRELVM vm);

        static SQInteger setSectionScale(HSQUIRRELVM vm);


        static void _getTextureStrings(HSQUIRRELVM vm, const SQChar** name, const SQChar** group);

    };
}
