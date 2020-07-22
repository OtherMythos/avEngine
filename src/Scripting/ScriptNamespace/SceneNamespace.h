#pragma once

#include "ScriptUtils.h"

namespace Ogre{
    class SceneManager;
}

namespace AV{
    class ScriptVM;

    /**
    A namespace providing access to aspects of the scene.
    This includes managing nodes.
    */
    class SceneNamespace{
        friend ScriptVM;
    public:
        SceneNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger getRootSceneNode(HSQUIRRELVM vm);
        static SQInteger createItem(HSQUIRRELVM vm);

        static Ogre::SceneManager* _scene;
    };
}
