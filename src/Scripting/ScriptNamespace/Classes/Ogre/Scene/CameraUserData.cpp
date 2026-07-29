#include "CameraUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"
#include "MovableObjectUserData.h"
#include "RayUserData.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptNamespace/Classes/QuaternionUserData.h"

#include "OgreMovableObject.h"
#include "OgreCamera.h"
#include "OgreRay.h"

namespace AV{

    SQObject CameraUserData::cameraDelegateTableObject;

    void CameraUserData::cameraToUserData(HSQUIRRELVM vm, Ogre::Camera* cam){
        Ogre::MovableObject** pointer = (Ogre::MovableObject**)sq_newuserdata(vm, sizeof(Ogre::MovableObject*));
        *pointer = (Ogre::MovableObject*)cam;

        sq_pushobject(vm, cameraDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, CameraTypeTag);
    }

    Ogre::Camera* CameraUserData::_readCamera(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(MovableObjectUserData::readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Camera));
        Ogre::Camera* cam = dynamic_cast<Ogre::Camera*>(outObject);
        assert(cam);

        return cam;
    }

    SQInteger CameraUserData::cameraLookAt(HSQUIRRELVM vm, Ogre::Camera* cam){
        Ogre::Vector3 target;
        SQInteger result = ScriptGetterUtils::vector3Read(vm, &target);
        if(result != 0) return result;

        cam->lookAt(target);

        return 0;
    }

    SQInteger CameraUserData::cameraSetProjectionType(HSQUIRRELVM vm, Ogre::Camera* cam){
        SQInteger proj;
        sq_getinteger(vm, 2, &proj);

        if(proj != Ogre::ProjectionType::PT_PERSPECTIVE && proj != Ogre::ProjectionType::PT_ORTHOGRAPHIC){
           return sq_throwerror(vm, "Invalid projection type provided.");
        }

        Ogre::ProjectionType projType = static_cast<Ogre::ProjectionType>(proj);
        cam->setProjectionType(projType);

        return 0;
    }

    SQInteger CameraUserData::cameraSetOrthoWindow(HSQUIRRELVM vm, Ogre::Camera* cam){
        SQFloat w, h;
        sq_getfloat(vm, 2, &w);
        sq_getfloat(vm, 3, &h);

        cam->setOrthoWindow(w, h);

        return 0;
    }

    SQInteger CameraUserData::cameraGetWorldPosInWindow(HSQUIRRELVM vm, Ogre::Camera* cam){
        Ogre::Vector3 target;
        SQInteger result = ScriptGetterUtils::vector3Read(vm, &target);
        if(result != 0) return result;

        Ogre::Vector3 viewPos = cam->getViewMatrix() * target;
        if(viewPos.z >= 0.0f){
            sq_pushnull(vm);
            return 1;
        }

        Ogre::Vector3 hcsPosition = cam->getProjectionMatrix() * viewPos;
        Vector3UserData::vector3ToUserData(vm, hcsPosition);

        return 1;
    }

    SQInteger CameraUserData::cameraSetAspectRatio(HSQUIRRELVM vm, Ogre::Camera* cam){
        SQFloat aspectRatio;
        sq_getfloat(vm, 2, &aspectRatio);
        if(aspectRatio <= 0) return sq_throwerror(vm, "Camera aspect ratio must be greater than 0.");

        cam->setAspectRatio(aspectRatio);

        return 0;
    }

    SQInteger CameraUserData::cameraSetFOVy(HSQUIRRELVM vm, Ogre::Camera* cam){
        SQFloat fovyRadians;
        sq_getfloat(vm, 2, &fovyRadians);
        if(fovyRadians <= 0) return sq_throwerror(vm, "Camera FOVy must be greater than 0.");

        cam->setFOVy(Ogre::Radian(fovyRadians));

        return 0;
    }

    SQInteger CameraUserData::cameraGetCameraToViewportRay(HSQUIRRELVM vm, Ogre::Camera* cam){
        SQFloat x, y;
        sq_getfloat(vm, 2, &x);
        sq_getfloat(vm, 3, &y);

        Ogre::Ray ray = cam->getCameraToViewportRay(x, y);
        RayUserData::RayToUserData(vm, &ray);

        return 1;
    }

    SQInteger CameraUserData::cameraSetDirection(HSQUIRRELVM vm, Ogre::Camera* cam){
        Ogre::Vector3 target;
        SQInteger result = ScriptGetterUtils::vector3Read(vm, &target);
        if(result != 0) return result;

        cam->setDirection(target);

        return 0;
    }

    SQInteger CameraUserData::cameraGetOrientation(HSQUIRRELVM vm, Ogre::Camera* cam){
        const Ogre::Quaternion orientation = cam->getOrientation();
        QuaternionUserData::quaternionToUserData(vm, orientation);

        return 1;
    }

    SQInteger CameraUserData::cameraSetNearClipDistance(HSQUIRRELVM vm, Ogre::Camera* cam){
        SQFloat nearValue;
        sq_getfloat(vm, 2, &nearValue);

        cam->setNearClipDistance(nearValue);

        return 0;
    }

    SQInteger CameraUserData::cameraSetFarClipDistance(HSQUIRRELVM vm, Ogre::Camera* cam){
        SQFloat farValue;
        sq_getfloat(vm, 2, &farValue);

        cam->setFarClipDistance(farValue);

        return 0;
    }

    SQInteger CameraUserData::_lookAt(HSQUIRRELVM vm){
        return cameraLookAt(vm, _readCamera(vm));
    }

    SQInteger CameraUserData::_setProjectionType(HSQUIRRELVM vm){
        return cameraSetProjectionType(vm, _readCamera(vm));
    }

    SQInteger CameraUserData::_setOrthoWindow(HSQUIRRELVM vm){
        return cameraSetOrthoWindow(vm, _readCamera(vm));
    }

    SQInteger CameraUserData::_getWorldPosInWindow(HSQUIRRELVM vm){
        return cameraGetWorldPosInWindow(vm, _readCamera(vm));
    }

    SQInteger CameraUserData::_setAspectRatio(HSQUIRRELVM vm){
        return cameraSetAspectRatio(vm, _readCamera(vm));
    }

    SQInteger CameraUserData::_getCameraToViewportRay(HSQUIRRELVM vm){
        return cameraGetCameraToViewportRay(vm, _readCamera(vm));
    }

    SQInteger CameraUserData::_setDirection(HSQUIRRELVM vm){
        return cameraSetDirection(vm, _readCamera(vm));
    }

    SQInteger CameraUserData::_getOrientation(HSQUIRRELVM vm){
        return cameraGetOrientation(vm, _readCamera(vm));
    }

    SQInteger CameraUserData::_setNearClipDistance(HSQUIRRELVM vm){
        return cameraSetNearClipDistance(vm, _readCamera(vm));
    }

    SQInteger CameraUserData::_setFarClipDistance(HSQUIRRELVM vm){
        return cameraSetFarClipDistance(vm, _readCamera(vm));
    }

    SQInteger CameraUserData::_setFOVy(HSQUIRRELVM vm){
        return cameraSetFOVy(vm, _readCamera(vm));
    }

    void CameraUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, _lookAt, "lookAt", -2, ".n|unn");
        ScriptUtils::addFunction(vm, MovableObjectUserData::getParentNode, "getParentNode");
        ScriptUtils::addFunction(vm, _setProjectionType, "setProjectionType", 2, ".i");
        ScriptUtils::addFunction(vm, _setOrthoWindow, "setOrthoWindow", 3, ".nn");
        ScriptUtils::addFunction(vm, _getWorldPosInWindow, "getWorldPosInWindow", -2, ".n|unn");
        ScriptUtils::addFunction(vm, _setAspectRatio, "setAspectRatio", 2, ".n");
        ScriptUtils::addFunction(vm, _getCameraToViewportRay, "getCameraToViewportRay", 3, ".nn");
        ScriptUtils::addFunction(vm, _setDirection, "setDirection", -2, ".n|unn");
        ScriptUtils::addFunction(vm, _getOrientation, "getOrientation");
        ScriptUtils::addFunction(vm, _setNearClipDistance, "setNearClipDistance", 2, ".n");
        ScriptUtils::addFunction(vm, _setFarClipDistance, "setFarClipDistance", 2, ".n");
        ScriptUtils::addFunction(vm, _setFOVy, "setFOVy", 2, ".n");

        sq_resetobject(&cameraDelegateTableObject);
        sq_getstackobj(vm, -1, &cameraDelegateTableObject);
        sq_addref(vm, &cameraDelegateTableObject);
        sq_pop(vm, 1);
    }
}
