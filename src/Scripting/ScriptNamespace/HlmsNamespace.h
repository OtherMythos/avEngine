#pragma once

#include "ScriptUtils.h"
#include "OgreHlms.h"

namespace Ogre{
    struct HlmsMacroblock;
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

        static SQInteger getDatablock(HSQUIRRELVM vm);
        static SQInteger destroyDatablock(HSQUIRRELVM vm);

        static void _parseMacroblockConstructionInfo(HSQUIRRELVM vm, Ogre::HlmsMacroblock* block);
        static SQInteger _getDefaultDatablock(HSQUIRRELVM vm, Ogre::HlmsTypes type);
    };
}
