#pragma once

#include "OgreVector3.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace Ogre{
    class HlmsPbsDatablock;
}

namespace AV{
    class DatablockPbsDelegate{
    public:
        DatablockPbsDelegate() = delete;

        static void setupTable(HSQUIRRELVM vm);

    private:
        static SQInteger setDiffuse(HSQUIRRELVM vm);
        static SQInteger setMetalness(HSQUIRRELVM vm);
        static SQInteger setRoughness(HSQUIRRELVM vm);
        static SQInteger setEmissive(HSQUIRRELVM vm);
        static SQInteger setFresnel(HSQUIRRELVM vm);
        static SQInteger setSpecular(HSQUIRRELVM vm);
        static SQInteger setTransparency(HSQUIRRELVM vm);
        static SQInteger setWorkflow(HSQUIRRELVM vm);
        static SQInteger setTexture(HSQUIRRELVM vm);
        static SQInteger setTextureUVSource(HSQUIRRELVM vm);

        static SQInteger getTransparency(HSQUIRRELVM vm);
        static SQInteger getTransparencyMode(HSQUIRRELVM vm);
        static SQInteger getUseAlphaFromTextures(HSQUIRRELVM vm);
        static SQInteger getWorkflow(HSQUIRRELVM vm);

        static void _getVector3(HSQUIRRELVM vm, Ogre::HlmsPbsDatablock*& db, Ogre::Vector3& vec);
        static void _getPbsBlock(HSQUIRRELVM vm, Ogre::HlmsPbsDatablock** db, SQInteger idx);
    };
}
