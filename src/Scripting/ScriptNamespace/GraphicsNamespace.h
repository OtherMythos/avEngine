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
    };
}
