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

    /**SQNamespace
    @name _camera
    @desc Namespace to alter the camera.
    */
    void CameraNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name setPosition
        @desc Set the position of the camera.
        @param1:x: x position.
        @param2:y: y position.
        @param3:z: x position.
        */
        ScriptUtils::addFunction(vm, setCameraPosition, "setPosition", 4, ".nnn");
        /**SQFunction
        @name lookAt
        @desc Point the camera towards a specific coordinate.
        @param1:x: Point x position.
        @param2:y: Point y position.
        @param3:z: Point x position.
        */
        ScriptUtils::addFunction(vm, cameraLookat, "lookAt", 4, ".nnn");
    }
}
