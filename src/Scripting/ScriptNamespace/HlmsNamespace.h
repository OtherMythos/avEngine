#pragma once

#include "ScriptUtils.h"
#include "OgreHlms.h"

namespace Ogre{
    struct HlmsMacroblock;
    struct HlmsSamplerblock;
}

namespace AV {
    class HlmsNamespace{
    public:
        HlmsNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

    private:
        static SQInteger PBSCreateDatablock(HSQUIRRELVM vm);
        static SQInteger PBSGetDefaultDatablock(HSQUIRRELVM vm);

        static SQInteger UnlitCreateDatablock(HSQUIRRELVM vm);
        static SQInteger UnlitGetDefaultDatablock(HSQUIRRELVM vm);
        static SQInteger getMacroblock(HSQUIRRELVM vm);
        static SQInteger getBlendblock(HSQUIRRELVM vm);
        static SQInteger getSamplerblock(HSQUIRRELVM vm);

        static SQInteger getDatablock(HSQUIRRELVM vm);
        static SQInteger destroyDatablock(HSQUIRRELVM vm);

        static SQInteger _createPassBuffer(HSQUIRRELVM vm, Ogre::HlmsTypes type);
        static SQInteger _getPassBuffer(HSQUIRRELVM vm, Ogre::HlmsTypes type);
        static SQInteger _getPass(HSQUIRRELVM vm, Ogre::HlmsTypes type);
        static SQInteger PBSCreatePassBuffer(HSQUIRRELVM vm);
        static SQInteger PBSGetPassBuffer(HSQUIRRELVM vm);
        static SQInteger PBSGetPass(HSQUIRRELVM vm);
        static SQInteger UnlitCreatePassBuffer(HSQUIRRELVM vm);
        static SQInteger UnlitGetPassBuffer(HSQUIRRELVM vm);
        static SQInteger UnlitGetPass(HSQUIRRELVM vm);

        static void _parseMacroblockConstructionInfo(HSQUIRRELVM vm, Ogre::HlmsMacroblock* block);
        static void _parseBlendblockConstructionInfo(HSQUIRRELVM vm, Ogre::HlmsBlendblock* block);
        static void _parseSamplerblockConstructionInfo(HSQUIRRELVM vm, Ogre::HlmsSamplerblock* block);
        static SQInteger _getDefaultDatablock(HSQUIRRELVM vm, Ogre::HlmsTypes type);
        static bool _getSceneBlendOperation(SQInteger value, Ogre::SceneBlendOperation* out);
        static bool _getSceneBlendFactor(SQInteger value, Ogre::SceneBlendFactor* out);
    };
}
