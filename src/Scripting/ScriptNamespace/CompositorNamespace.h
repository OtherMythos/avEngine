#pragma once

#include "ScriptUtils.h"

namespace Ogre{
    class SceneManager;
};

namespace AV{
    class ScriptVM;
    class CompositorNamespace{
        friend ScriptVM;
    public:
        CompositorNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static Ogre::SceneManager* _scene;

        static SQInteger addWorkspace(HSQUIRRELVM vm);
    };
}
