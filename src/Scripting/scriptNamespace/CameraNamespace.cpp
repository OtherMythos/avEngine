#include "CameraNamespace.h"

#include "Logger/Log.h"
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
    }

    SQInteger CameraNamespace::cameraLookat(HSQUIRRELVM vm){
        SQFloat x, y, z;
        sq_getfloat(vm, -1, &z);
        sq_getfloat(vm, -2, &y);
        sq_getfloat(vm, -3, &x);

        _camera->lookAt(Ogre::Vector3(x, y, z));

        sq_pop(vm, 3);
    }

    void CameraNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, setCameraPosition, "setPosition");
        _addFunction(vm, cameraLookat, "lookAt");
    }
}
