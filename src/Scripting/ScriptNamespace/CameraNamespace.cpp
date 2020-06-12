#include "CameraNamespace.h"

#include "OgreCamera.h"
#include "OgreVector3.h"

#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"

namespace AV{
    Ogre::Camera* CameraNamespace::_camera = 0;

    bool _vec3Read(HSQUIRRELVM vm, Ogre::Vector3& outVec){
        SQInteger size = sq_gettop(vm);

        if(size == 2){
            //Vector3
            if(Vector3UserData::readVector3FromUserData(vm, -1, &outVec)) return true;

            //TODO there should be error checking here.
            SlotPosition pos = SlotPositionClass::getSlotFromInstance(vm, -1);
            outVec = pos.toOgre();

            //return false;
        }else if(size == 4){
            //Regular

            bool success = true;
            SQFloat x, y, z;
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -1, &z));
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -2, &y));
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -3, &x));
            if(!success) return false;

            outVec = Ogre::Vector3(x, y, z);

            sq_pop(vm, 3);
        }else return false;

        return true;
    }

    SQInteger CameraNamespace::setCameraPosition(HSQUIRRELVM vm){
        Ogre::Vector3 target;
        if(!_vec3Read(vm, target)){
            return 0;
        }

        _camera->setPosition(target);

        return 0;
    }

    SQInteger CameraNamespace::cameraLookat(HSQUIRRELVM vm){
        Ogre::Vector3 target;
        if(!_vec3Read(vm, target)){
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
        ScriptUtils::addFunction(vm, setCameraPosition, "setPosition", -2, ".n|u|xnn");
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
        ScriptUtils::addFunction(vm, cameraLookat, "lookAt", -2, ".n|u|xnn");
    }
}
