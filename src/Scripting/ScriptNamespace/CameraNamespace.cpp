#include "CameraNamespace.h"

#include "OgreCamera.h"
#include "OgreVector3.h"
#include "ScriptGetterUtils.h"

namespace AV{
    Ogre::Camera* CameraNamespace::_camera = 0;

    SQInteger CameraNamespace::setCameraPosition(HSQUIRRELVM vm){
        Ogre::Vector3 target;
        if(!ScriptGetterUtils::vector3Read(vm, &target)){
            return 0;
        }

        _camera->setPosition(target);

        return 0;
    }

    SQInteger CameraNamespace::cameraLookat(HSQUIRRELVM vm){
        Ogre::Vector3 target;
        if(!ScriptGetterUtils::vector3Read(vm, &target)){
            return 0;
        }

        _camera->lookAt(target);

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
        /**SQFunction
        @name setPosition
        @desc Set the position of the camera.
        @param1:Vec3: Vector representing the position in 3d space.
        */
        /**SQFunction
        @name setPosition
        @desc Set the position of the camera.
        @param1:SlotPosition: SlotPosition representing the position in world coordinates. This will be resolved to local coordinates automatically.
        */
        ScriptUtils::addFunction(vm, setCameraPosition, "setPosition", -2, ".n|unn");
        /**SQFunction
        @name lookAt
        @desc Point the camera towards a specific coordinate.
        @param1:x: Point x position.
        @param2:y: Point y position.
        @param3:z: Point x position.
        */
        /**SQFunction
        @name lookAt
        @desc Point the camera towards a specific coordinate.
        @param1:Vec3: Vector representing the position to look at.
        */
        /**SQFunction
        @name lookAt
        @desc Point the camera towards a specific coordinate.
        @param1:SlotPosition: SlotPosition representing the position in world coordinates. This will be resolved to local coordinates automatically.
        */
        ScriptUtils::addFunction(vm, cameraLookat, "lookAt", -2, ".n|unn");
    }
}
