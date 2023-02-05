#include "CameraNamespace.h"

#include "OgreCamera.h"
#include "OgreVector3.h"
#include "OgreRay.h"
#include "ScriptGetterUtils.h"
#include "Scripting/ScriptNamespace/Classes/QuaternionUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/RayUserData.h"

#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/MovableObjectUserData.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"

#include "System/EngineFlags.h"

namespace AV{
    Ogre::Camera* CameraNamespace::_camera = 0;

    SQInteger CameraNamespace::setCameraPosition(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN();
        Ogre::Vector3 target;
        SQInteger result = ScriptGetterUtils::vector3Read(vm, &target);
        if(result != 0) return result;

        _camera->setPosition(target);

        return 0;
    }

    SQInteger CameraNamespace::getCameraPosition(HSQUIRRELVM vm){

        Ogre::Vector3 pos = _camera->getPosition();
        Vector3UserData::vector3ToUserData(vm, pos);

        return 1;
    }

    SQInteger CameraNamespace::getCameraOrientation(HSQUIRRELVM vm){

        Ogre::Quaternion orientation = _camera->getOrientation();
        QuaternionUserData::quaternionToUserData(vm, orientation);

        return 1;
    }

    SQInteger CameraNamespace::cameraLookat(HSQUIRRELVM vm){
        Ogre::Vector3 target;
        SQInteger result = ScriptGetterUtils::vector3Read(vm, &target);
        if(result != 0) return result;

        _camera->lookAt(target);

        return 0;
    }

    SQInteger CameraNamespace::setOrientation(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
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

    SQInteger CameraNamespace::setDirection(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
        Ogre::Vector3 target;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &target));

        _camera->setDirection(target);

        return 0;
    }

    SQInteger CameraNamespace::setProjectionType(HSQUIRRELVM vm){
        SQInteger proj;
        sq_getinteger(vm, 2, &proj);

        if(proj != Ogre::ProjectionType::PT_PERSPECTIVE && proj != Ogre::ProjectionType::PT_ORTHOGRAPHIC){
           return sq_throwerror(vm, "Invalid projection type provided.");
       }

        Ogre::ProjectionType projType = static_cast<Ogre::ProjectionType>(proj);

        _camera->setProjectionType(projType);

        return 0;
    }

    SQInteger CameraNamespace::setOrthoWindow(HSQUIRRELVM vm){
        SQFloat w, h;
        sq_getfloat(vm, 2, &w);
        sq_getfloat(vm, 3, &h);

        _camera->setOrthoWindow(w, h);

        return 0;
    }

    SQInteger CameraNamespace::getDirection(HSQUIRRELVM vm){
        Ogre::Vector3 targetDirection = _camera->getDirection();
        Vector3UserData::vector3ToUserData(vm, targetDirection);

        return 1;
    }

    SQInteger CameraNamespace::getDefaultCamera(HSQUIRRELVM vm){
        MovableObjectUserData::movableObjectToUserData(vm, (Ogre::MovableObject*)_camera, MovableObjectType::Camera);

        return 1;
    }


    SQInteger CameraNamespace::setAspectRatio(HSQUIRRELVM vm){
        SQFloat aspectRatio;
        sq_getfloat(vm, 2, &aspectRatio);
        if(aspectRatio <= 0) return sq_throwerror(vm, "Camera aspect ratio must be greater than 0.");

        _camera->setAspectRatio(aspectRatio);

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
        @name getPosition
        @desc Get the position of the camera as a vector3.
        */
        ScriptUtils::addFunction(vm, getCameraPosition, "getPosition");
        /**SQFunction
        @name getOrientation
        @desc Get the orientation of the camera as a quaternion.
        */
        ScriptUtils::addFunction(vm, getCameraOrientation, "getOrientation");
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
        /**SQFunction
        @name setDirection
        @desc Set the direction the camera faces.
        @param1:Vector3: A direction vector.
        */
        ScriptUtils::addFunction(vm, setDirection, "setDirection");
        /**SQFunction
        @name getDirection
        @desc Get the direction the camera faces.
        @returns A normalised direction vector.
        */
        ScriptUtils::addFunction(vm, getDirection, "getDirection");
        /**SQFunction
        @name setProjectionType
        @desc Set the projection type to use for this camera.
        */
        ScriptUtils::addFunction(vm, setProjectionType, "setProjectionType", 2, ".i");
        /**SQFunction
        @name setOrthoWindow
        @desc Sets the orthographic window settings, for use with orthographic rendering only.
        */
        ScriptUtils::addFunction(vm, setOrthoWindow, "setOrthoWindow", 3, ".nn");
        /**SQFunction
        @name setAspectRatio
        @desc set the aspect ration of the camera. Generally will be width / height.
        */
        ScriptUtils::addFunction(vm, setAspectRatio, "setAspectRatio", 2, ".n");
    }

    void CameraNamespace::setupConstants(HSQUIRRELVM vm){
        sq_pushroottable(vm);

        ScriptUtils::declareConstant(vm, "_PT_ORTHOGRAPHIC", (SQInteger)Ogre::ProjectionType::PT_ORTHOGRAPHIC);
        ScriptUtils::declareConstant(vm, "_PT_PERSPECTIVE", (SQInteger)Ogre::ProjectionType::PT_PERSPECTIVE);

        sq_pop(vm, 1);
    }
}
