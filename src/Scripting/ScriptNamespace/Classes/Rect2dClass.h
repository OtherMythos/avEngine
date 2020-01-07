#pragma once

#include <squirrel.h>

#include "Scripting/ScriptDataPacker.h"
#include "Gui/Rect2d/Rect2dManager.h"

namespace AV{

    /**
    A class to expose 2d rectangles to squirrel.
    */
    class Rect2dClass{
    public:
        Rect2dClass() = delete;

        static void setupClass(HSQUIRRELVM vm);

    private:
        static ScriptDataPacker<Rect2dPtr> mRects;
        static SQObject classObject;

        static SQInteger sqRectReleaseHook(SQUserPointer p, SQInteger size);

        static SQInteger movableTextureConstructor(HSQUIRRELVM vm);

        static SQInteger setRectPosition(HSQUIRRELVM vm);

        static SQInteger setRectWidth(HSQUIRRELVM vm);
        static SQInteger setRectHeight(HSQUIRRELVM vm);
        static SQInteger setRectSize(HSQUIRRELVM vm);
        static SQInteger setRectVisible(HSQUIRRELVM vm);

        static SQInteger setLayer(HSQUIRRELVM vm);
        static SQInteger getLayer(HSQUIRRELVM vm);

        static SQInteger setDatablock(HSQUIRRELVM vm);

    };
}
