#pragma once

#include "ScriptUtils.h"
#include "Classes/Ogre/Scene/OgreObjectTypes.h"

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
        static SQInteger testRayForSlot(HSQUIRRELVM vm);
        static SQInteger testRayForObject(HSQUIRRELVM vm);
        static SQInteger testRayForObjectArray(HSQUIRRELVM vm);
        static SQInteger createParticleSystem(HSQUIRRELVM vm);
        static SQInteger createCamera(HSQUIRRELVM vm);
        static SQInteger setAmbientLight(HSQUIRRELVM vm);

        static SQInteger registerChunkCallback(HSQUIRRELVM vm);
        static SQInteger getNumDataPoints(HSQUIRRELVM vm);
        static SQInteger getDataPointAt(HSQUIRRELVM vm);
        static SQInteger insertSceneFile(HSQUIRRELVM vm);

        static SQInteger createTagPoint(HSQUIRRELVM vm);

        static Ogre::SceneManager* _scene;
    };
}
