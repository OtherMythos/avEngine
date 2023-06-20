#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "Ogre.h"

namespace AV{
    class TextureBoxUserData{
    public:
        TextureBoxUserData() = delete;
        ~TextureBoxUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void TextureBoxToUserData(HSQUIRRELVM vm, Ogre::TextureBox* texBox);

        static UserDataGetResult readTextureBoxFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::TextureBox** outBox);

    private:
        static SQObject TextureBoxDelegateTableObject;

        static SQInteger getSizeBytes(HSQUIRRELVM vm);
        static SQInteger getColourAt(HSQUIRRELVM vm);
        static SQInteger writeVal(HSQUIRRELVM vm);

        static SQInteger Write(Ogre::TextureBox* box, void* buffer, SQInteger size);

        static SQInteger TextureBoxObjectReleaseHook(SQUserPointer p, SQInteger size);
    };
}
