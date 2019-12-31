#include "CameraNamespace.h"

#include "OgreCamera.h"
#include "OgreVector3.h"

namespace AV{
    Ogre::Camera* CameraNamespace::_camera = 0;

    SQInteger CameraNamespace::setCameraPosition(HSQUIRRELVM vm){
        SQFloat x, y, z;
        sq_getfloat(vm, -1, &z);
        sq_getfloat(vm, -2, &y);
        sq_getfloat(vm, -3, &x);

        _camera->setPosition(Ogre::Vector3(x, y, z));

        sq_pop(vm, 3);

        return 0;
    }

    SQInteger CameraNamespace::cameraLookat(HSQUIRRELVM vm){
        SQFloat x, y, z;
        sq_getfloat(vm, -1, &z);
        sq_getfloat(vm, -2, &y);
        sq_getfloat(vm, -3, &x);

        _camera->lookAt(Ogre::Vector3(x, y, z));

        sq_pop(vm, 3);

        return 0;
    }

    void CameraNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, setCameraPosition, "setPosition", 4, ".nnn");
        ScriptUtils::addFunction(vm, cameraLookat, "lookAt", 4, ".nnn");
    }
}
