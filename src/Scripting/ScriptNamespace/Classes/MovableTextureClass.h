#pragma once

#include <squirrel.h>

#include "System/Util/DataPacker.h"
#include "Gui/Rect2d/Rect2dManager.h"

namespace AV{

    /**
    A class to expose movable textures to squirrel.
    */
    class MovableTextureClass{
    public:
        MovableTextureClass() = delete;

        static void setupClass(HSQUIRRELVM vm);

        //Intended to be called by the testModeTextureNamespace.
        static bool isTextureInLayer(void* p, Rect2dManager::LayerId layer);

    private:
        static DataPacker<MovableTexturePtr> mTextures;
        static SQObject classObject;

        static SQInteger sqTextureReleaseHook(SQUserPointer p, SQInteger size);

        static SQInteger movableTextureConstructor(HSQUIRRELVM vm);

        static SQInteger setTexturePosition(HSQUIRRELVM vm);

        static SQInteger setTextureWidth(HSQUIRRELVM vm);
        static SQInteger setTextureHeight(HSQUIRRELVM vm);
        static SQInteger setTextureSize(HSQUIRRELVM vm);

        static SQInteger setTexture(HSQUIRRELVM vm);
        static SQInteger setTextureVisible(HSQUIRRELVM vm);

        static SQInteger setLayer(HSQUIRRELVM vm);
        static SQInteger getLayer(HSQUIRRELVM vm);

        static SQInteger setColour(HSQUIRRELVM vm);

        static SQInteger getPosition(HSQUIRRELVM vm);
        static SQInteger getSize(HSQUIRRELVM vm);

        static SQInteger setSectionScale(HSQUIRRELVM vm);


        static void _getTextureStrings(HSQUIRRELVM vm, std::string& name, std::string& group);

    };
}
