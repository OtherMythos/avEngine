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
        static void setupConstants(HSQUIRRELVM vm);

    private:
        static SQInteger createTexture(HSQUIRRELVM vm);
        static SQInteger getStagingTexture(HSQUIRRELVM vm);
        static SQInteger createOrRetreiveTexture(HSQUIRRELVM vm);
        static SQInteger destroyTexture(HSQUIRRELVM vm);
        static SQInteger getLoadedTextures(HSQUIRRELVM vm);
        static SQInteger getLoadedMeshes(HSQUIRRELVM vm);
        static SQInteger getGpuProgramByName(HSQUIRRELVM vm);
        static SQInteger getMaterialByName(HSQUIRRELVM vm);
        static SQInteger createManualMesh(HSQUIRRELVM vm);
        static SQInteger removeManualMesh(HSQUIRRELVM vm);
        static SQInteger createVertexBuffer(HSQUIRRELVM vm);
        static SQInteger createIndexBuffer(HSQUIRRELVM vm);
        static SQInteger createVertexElementVec(HSQUIRRELVM vm);
        static SQInteger createVertexArrayObject(HSQUIRRELVM vm);
        static SQInteger genPerlinNoiseTexture(HSQUIRRELVM vm);
    };
}
