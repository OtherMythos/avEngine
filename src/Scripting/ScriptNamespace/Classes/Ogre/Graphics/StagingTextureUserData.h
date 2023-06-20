#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "Ogre.h"

namespace AV{
    class StagingTextureUserData{
    public:
        StagingTextureUserData() = delete;
        ~StagingTextureUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void StagingTextureToUserData(HSQUIRRELVM vm, Ogre::StagingTexture* texBox);

        static UserDataGetResult readStagingTextureFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::StagingTexture** outBox);

    private:
        static SQObject StagingTextureDelegateTableObject;

        static SQInteger startMapRegion(HSQUIRRELVM vm);
        static SQInteger stopMapRegion(HSQUIRRELVM vm);
        static SQInteger mapRegion(HSQUIRRELVM vm);
        static SQInteger upload(HSQUIRRELVM vm);

        static SQInteger StagingTextureObjectReleaseHook(SQUserPointer p, SQInteger size);
    };
}
