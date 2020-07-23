#pragma once

#include "ScriptUtils.h"
#include "Classes/Ogre/OgreObjectTypes.h"

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

        static MovableObjectType determineTypeFromMovableObject(const Ogre::MovableObject* obj);

    private:
        static SQInteger getRootSceneNode(HSQUIRRELVM vm);
        static SQInteger createItem(HSQUIRRELVM vm);
        static SQInteger createLight(HSQUIRRELVM vm);

        static Ogre::SceneManager* _scene;
    };
}
