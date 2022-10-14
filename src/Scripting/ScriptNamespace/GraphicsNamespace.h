#pragma once

#include "ScriptUtils.h"

namespace Ogre{
    class SceneManager;
};

namespace AV{
    class ScriptVM;
    class GraphicsNamespace{
        friend ScriptVM;
    public:
        GraphicsNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger createTexture(HSQUIRRELVM vm);
        static SQInteger createOrRetreiveTexture(HSQUIRRELVM vm);
        static SQInteger destroyTexture(HSQUIRRELVM vm);
        static SQInteger getLoadedTextures(HSQUIRRELVM vm);
        static SQInteger getLoadedMeshes(HSQUIRRELVM vm);
        static SQInteger getGpuProgramByName(HSQUIRRELVM vm);
    };
}
