#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "Ogre.h"

namespace AV{
    class TextureBoxUserData{
    private:
        struct TextureBoxWrapper;
    public:
        TextureBoxUserData() = delete;
        ~TextureBoxUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void TextureBoxToUserData(HSQUIRRELVM vm, Ogre::TextureBox* texBox);

        static UserDataGetResult readTextureBoxFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::TextureBox** outBox);
    private:
        static SQObject TextureBoxDelegateTableObject;

        static SQInteger seek(HSQUIRRELVM vm);
        static SQInteger tell(HSQUIRRELVM vm);
        static SQInteger getSizeBytes(HSQUIRRELVM vm);
        static SQInteger getColourAt(HSQUIRRELVM vm);
        static SQInteger writeVal(HSQUIRRELVM vm);

        static SQInteger Write(HSQUIRRELVM vm, TextureBoxWrapper* wrapper, void* buffer, size_t size);
        static SQInteger _checkPointerOverflow(HSQUIRRELVM vm, TextureBoxWrapper* wrapper, size_t size);
        static SQInteger _checkPointerOverflowFromStart(HSQUIRRELVM vm, TextureBoxWrapper* wrapper, size_t size);

        static UserDataGetResult _readTextureBoxFromUserData(HSQUIRRELVM vm, SQInteger stackInx, TextureBoxWrapper** outBox);
        static SQInteger TextureBoxObjectReleaseHook(SQUserPointer p, SQInteger size);
    };
}
