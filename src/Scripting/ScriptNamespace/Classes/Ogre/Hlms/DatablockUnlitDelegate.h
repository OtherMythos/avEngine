#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace Ogre{
    class HlmsUnlitDatablock;
}

namespace AV{
    class DatablockUnlitDelegate{
    public:
        DatablockUnlitDelegate() = delete;

        static void setupTable(HSQUIRRELVM vm);

    private:
        static SQInteger setColour(HSQUIRRELVM vm);
        static SQInteger setUseColour(HSQUIRRELVM vm);
        static SQInteger setTexture(HSQUIRRELVM vm);
        static SQInteger getColour(HSQUIRRELVM vm);
        static SQInteger setAnimationMatrix(HSQUIRRELVM vm);
        static SQInteger setEnableAnimationMatrix(HSQUIRRELVM vm);

        static void _getUnitBlock(HSQUIRRELVM vm, Ogre::HlmsUnlitDatablock** db, SQInteger idx);

        static SQInteger getTypeof(HSQUIRRELVM vm);
    };
}
