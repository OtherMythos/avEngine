#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "System/Util/VersionedPtr.h"

namespace Ogre{
    class TextureGpu;
}

namespace AV{
    class TextureUserData{
        friend class AVTextureGpuManagerListener;
    private:

        struct TextureUserDataContents{
            Ogre::TextureGpu* ptr;
            uint64 textureId;
            bool userOwned;
        };

    public:
        TextureUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

        static void textureToUserData(HSQUIRRELVM vm, Ogre::TextureGpu* tex, bool userOwned);

        static UserDataGetResult readTextureFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::TextureGpu** outObject, bool* userOwned);
        static UserDataGetResult readTextureFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::TextureGpu** outObject, bool* userOwned, bool* isValid);

        static void setupListener();

    private:

        static SQObject textureDelegateTableObject;
        static VersionedPtr<Ogre::TextureGpu*> _data;

        static SQInteger getWidth(HSQUIRRELVM vm);
        static SQInteger getHeight(HSQUIRRELVM vm);
        static SQInteger setResolution(HSQUIRRELVM vm);
        static SQInteger setPixelFormat(HSQUIRRELVM vm);
        static SQInteger schduleTransitionTo(HSQUIRRELVM vm);
        static SQInteger isTextureValid(HSQUIRRELVM vm);
        static SQInteger getName(HSQUIRRELVM vm);
        static SQInteger waitForData(HSQUIRRELVM vm);
        static SQInteger barrierSolveTexture(HSQUIRRELVM vm);

        static void _notifyTextureDeleted(Ogre::TextureGpu* texture);

        static SQInteger textureReleaseHook(SQUserPointer p, SQInteger size);

        static UserDataGetResult _readTexturePtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, TextureUserDataContents** outObject);

        static bool _isTexValid(const TextureUserDataContents* content);
    };
}
