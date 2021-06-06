#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "System/Util/VersionedPtr.h"
#include <map>

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

        static UserDataGetResult readTextureFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::TextureGpu** outObject);

        static void setupListener();

    private:

        static SQObject textureDelegateTableObject;
        static VersionedPtr<Ogre::TextureGpu*> _data;

        static SQInteger getWidth(HSQUIRRELVM vm);
        static SQInteger getHeight(HSQUIRRELVM vm);
        static SQInteger setResolution(HSQUIRRELVM vm);
        static SQInteger schduleTransitionTo(HSQUIRRELVM vm);

        static void _notifyTextureDeleted(Ogre::TextureGpu* texture);

        static UserDataGetResult _readTexturePtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, TextureUserDataContents** outObject);

    };
}
