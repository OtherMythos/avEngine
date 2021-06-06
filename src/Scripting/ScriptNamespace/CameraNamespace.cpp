#include "CameraNamespace.h"

#include "OgreCamera.h"
#include "OgreVector3.h"
#include "OgreRay.h"
#include "ScriptGetterUtils.h"
#include "Scripting/ScriptNamespace/Classes/QuaternionUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/RayUserData.h"

#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/MovableObjectUserData.h"

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

    SQInteger CameraNamespace::setOrientation(HSQUIRRELVM vm){
        Ogre::Quaternion outQuat;
        SCRIPT_CHECK_RESULT(QuaternionUserData::readQuaternionFromUserData(vm, 2, &outQuat));

        _camera->setOrientation(outQuat);

        return 0;
    }

    SQInteger CameraNamespace::getCameraToViewportRay(HSQUIRRELVM vm){
        SQFloat x, y;
        sq_getfloat(vm, -1, &y);
        sq_getfloat(vm, -2, &x);

        Ogre::Ray ray = _camera->getCameraToViewportRay(x, y);
        RayUserData::RayToUserData(vm, &ray);

        return 1;
    }

    SQInteger CameraNamespace::getDefaultCamera(HSQUIRRELVM vm){
        MovableObjectUserData::movableObjectToUserData(vm, (Ogre::MovableObject*)_camera, MovableObjectType::Camera);

        return 1;
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
        /**SQFunction
        @name setOrientation
        @desc Set the orientation of the camera.
        @param1:Quaternion: The quaternion orientation to set.
        */
        ScriptUtils::addFunction(vm, setOrientation, "setOrientation", 2, ".u");
        /**SQFunction
        @name getCameraToViewportRay
        @desc Obtain a ray pointing in a specific direction based on coordinates.
        @param1:x: A normalised float value for the x coordinates.
        @param1:y: A normalised float value for the y coordinates.
        */
        ScriptUtils::addFunction(vm, getCameraToViewportRay, "getCameraToViewportRay", 3, ".nn");
        /**SQFunction
        @name getCamera
        @desc Get the camera as an object. With this it can be used as part of compositor effects.
        @returns A camera object
        */
        ScriptUtils::addFunction(vm, getDefaultCamera, "getCamera");
    }
}
