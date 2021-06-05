#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "System/Util/VersionedDataPool.h"
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
        //Keep track of which textures are currently active as user data objects.
        //Really the value I'm storing isn't important, so store a count of how many objects reference this pointer.
        static VersionedDataPool<uint32> textureDataPool;
        //Keep track of which pointer references which entry in the texture pool.
        static std::map<Ogre::TextureGpu*, uint64> existingTextures;

        static SQInteger getWidth(HSQUIRRELVM vm);
        static SQInteger getHeight(HSQUIRRELVM vm);
        static SQInteger setResolution(HSQUIRRELVM vm);
        static SQInteger schduleTransitionTo(HSQUIRRELVM vm);

        static void _notifyTextureDeleted(Ogre::TextureGpu* texture);

        static UserDataGetResult _readTexturePtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, TextureUserDataContents** outObject);

    };
}
