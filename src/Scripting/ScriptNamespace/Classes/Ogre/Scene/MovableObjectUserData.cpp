#include "MovableObjectUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"

#include "AabbUserData.h"
#include "Skeleton/SkeletonUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Hlms/DatablockUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/RayUserData.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptNamespace/Classes/QuaternionUserData.h"
#include "OgreMovableObject.h"
#include "OgreItem.h"
#include "OgreLight.h"
#include "OgreCamera.h"
#include "OgreRay.h"

#include "Particles/ParticleSystemUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/SceneNodeUserData.h"

namespace AV{

    SQObject MovableObjectUserData::itemDelegateTableObject;
    SQObject MovableObjectUserData::lightDelegateTableObject;
    SQObject MovableObjectUserData::particleSystemDelegateTableObject;
    SQObject MovableObjectUserData::cameraDelegateTableObject;

    bool _typeTagMovableObject(void* tag){
        return tag >= (void*)71 && tag < (void*)80;
    }

    void MovableObjectUserData::movableObjectToUserData(HSQUIRRELVM vm, Ogre::MovableObject* object, MovableObjectType type){
        Ogre::MovableObject** pointer = (Ogre::MovableObject**)sq_newuserdata(vm, sizeof(Ogre::MovableObject*));
        *pointer = object;

        void* targetTypeTag = 0;
        SQObject* targetTable = 0;
        switch(type){
            case MovableObjectType::Item:
                targetTypeTag = MovableObjectItemTypeTag;
                targetTable = &itemDelegateTableObject;
                break;
            case MovableObjectType::Light:
                targetTypeTag = MovableObjectLightTypeTag;
                targetTable = &lightDelegateTableObject;
                break;
            case MovableObjectType::ParticleSystem:
                targetTypeTag = ParticleSystemTypeTag;
                targetTable = &particleSystemDelegateTableObject;
                break;
            case MovableObjectType::Camera:
                targetTypeTag = CameraTypeTag;
                targetTable = &cameraDelegateTableObject;
                break;
            default:
                assert(false);
                break;
        }
        //Write a thing to determine the delegate table. Create the other delegate table in the setup bit.
        sq_pushobject(vm, *targetTable);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, targetTypeTag);
    }

    UserDataGetResult MovableObjectUserData::readMovableObjectFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::MovableObject** outObject, MovableObjectType expectedType){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(expectedType == MovableObjectType::Any){
            if(!_typeTagMovableObject(typeTag)){
                *outObject = 0;
                return USER_DATA_GET_TYPE_MISMATCH;
            }
        }else{
            static void* const targetTags[] = {
                0,
                MovableObjectItemTypeTag,
                MovableObjectLightTypeTag,
                ParticleSystemTypeTag,
                CameraTypeTag
            };
            if(typeTag != targetTags[(size_t)expectedType]){
                *outObject = 0;
                return USER_DATA_GET_TYPE_MISMATCH;
            }
        }

        Ogre::MovableObject** p = (Ogre::MovableObject**)pointer;
        *outObject = *p;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger MovableObjectUserData::setDatablock(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Item));

        SQObjectType t = sq_gettype(vm, 2);
        Ogre::Item* targetItem = static_cast<Ogre::Item*>(outObject);
        if(t == OT_USERDATA){
            Ogre::HlmsDatablock* db = 0;
            SCRIPT_CHECK_RESULT(DatablockUserData::getPtrFromUserData(vm, 2, &db));

            targetItem->setDatablock(db);
        }else if(t == OT_STRING){
            const SQChar *dbPath;
            sq_getstring(vm, 2, &dbPath);

            targetItem->setDatablock(dbPath);
        }else{
            assert(false);
        }

        return 0;
    }

    SQInteger MovableObjectUserData::setLightType(HSQUIRRELVM vm){
        SQInteger idx = 0;
        sq_getinteger(vm, 2, &idx);
        if(idx < 0 || idx >= Ogre::Light::NUM_LIGHT_TYPES) return sq_throwerror(vm, "Invalid light type id");

        Ogre::MovableObject* outObj = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObj, MovableObjectType::Light));

        Ogre::Light* lightObj = static_cast<Ogre::Light*>(outObj);

        lightObj->setType( (Ogre::Light::LightTypes)idx);

        return 0;
    }

    SQInteger MovableObjectUserData::setLightPowerScale(HSQUIRRELVM vm){
        SQFloat power = 0;
        sq_getfloat(vm, 2, &power);

        Ogre::MovableObject* outObj = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObj, MovableObjectType::Light));

        Ogre::Light* lightObj = static_cast<Ogre::Light*>(outObj);

        lightObj->setPowerScale(power);

        return 0;
    }

    SQInteger MovableObjectUserData::setLightDiffuseColour(HSQUIRRELVM vm){
        SQFloat floatValues[3];
        ScriptUtils::getFloatValues<3>(vm, 2, floatValues);

        Ogre::MovableObject* outObj = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObj, MovableObjectType::Light));

        Ogre::Light* lightObj = static_cast<Ogre::Light*>(outObj);

        lightObj->setDiffuseColour(floatValues[0], floatValues[1], floatValues[2]);

        return 0;
    }

    SQInteger MovableObjectUserData::setLightSpecularColour(HSQUIRRELVM vm){
        SQFloat floatValues[3];
        ScriptUtils::getFloatValues<3>(vm, 2, floatValues);

        Ogre::MovableObject* outObj = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObj, MovableObjectType::Light));

        Ogre::Light* lightObj = static_cast<Ogre::Light*>(outObj);

        lightObj->setSpecularColour(floatValues[0], floatValues[1], floatValues[2]);

        return 0;
    }

    SQInteger MovableObjectUserData::setLightAttenuationBasedOnRadius(HSQUIRRELVM vm){
        SQFloat radius, lumThreshold;
        sq_getfloat(vm, 2, &radius);
        sq_getfloat(vm, 3, &lumThreshold);

        Ogre::MovableObject* outObj = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObj, MovableObjectType::Light));

        Ogre::Light* lightObj = static_cast<Ogre::Light*>(outObj);

        lightObj->setAttenuationBasedOnRadius(radius, lumThreshold);

        return 0;
    }

    SQInteger MovableObjectUserData::getLocalRadius(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Any));

        sq_pushfloat(vm, outObject->getLocalRadius());

        return 1;
    }

    SQInteger MovableObjectUserData::itemHasSkeleton(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Any));
        Ogre::Item* item = dynamic_cast<Ogre::Item*>(outObject);
        assert(item);

        sq_pushbool(vm, item->hasSkeleton());

        return 1;
    }

    SQInteger MovableObjectUserData::itemUseSkeletonInstanceFrom(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Item));
        Ogre::Item* item = dynamic_cast<Ogre::Item*>(outObject);
        assert(item);

        Ogre::MovableObject* outObjectSecond = 0;
        SCRIPT_CHECK_RESULT(readMovableObjectFromUserData(vm, 2, &outObjectSecond, MovableObjectType::Item));
        Ogre::Item* itemSecond = dynamic_cast<Ogre::Item*>(outObjectSecond);
        assert(itemSecond);

        item->useSkeletonInstanceFrom(itemSecond);

        return 1;
    }

    SQInteger MovableObjectUserData::itemGetSkeleton(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Any));
        Ogre::Item* item = dynamic_cast<Ogre::Item*>(outObject);
        assert(item);

        Ogre::SkeletonInstance* skeleton = item->getSkeletonInstance();
        SkeletonUserData::skeletonToUserData(vm, skeleton);

        return 1;
    }

    SQInteger MovableObjectUserData::getLocalAabb(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Any));

        Ogre::Aabb localAabb = outObject->getLocalAabb();
        AabbUserData::AabbToUserData(vm, &localAabb);

        return 1;
    }

    SQInteger MovableObjectUserData::cameraLookAt(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Camera));
        Ogre::Camera* cam = dynamic_cast<Ogre::Camera*>(outObject);
        assert(cam);

        Ogre::Vector3 target;
        SQInteger result = ScriptGetterUtils::vector3Read(vm, &target);
        if(result != 0) return result;

        cam->lookAt(target);

        return 0;
    }

    SQInteger MovableObjectUserData::cameraSetProjectionType(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Camera));
        Ogre::Camera* cam = dynamic_cast<Ogre::Camera*>(outObject);
        assert(cam);

        SQInteger proj;
        sq_getinteger(vm, 2, &proj);

        if(proj != Ogre::ProjectionType::PT_PERSPECTIVE && proj != Ogre::ProjectionType::PT_ORTHOGRAPHIC){
           return sq_throwerror(vm, "Invalid projection type provided.");
        }

        Ogre::ProjectionType projType = static_cast<Ogre::ProjectionType>(proj);
        cam->setProjectionType(projType);

        return 0;
    }

    SQInteger MovableObjectUserData::cameraSetOrthoWindow(HSQUIRRELVM vm){
        SQFloat w, h;
        sq_getfloat(vm, 2, &w);
        sq_getfloat(vm, 3, &h);

        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Camera));
        Ogre::Camera* cam = dynamic_cast<Ogre::Camera*>(outObject);
        assert(cam);
        cam->setOrthoWindow(w, h);

        return 0;
    }

    SQInteger MovableObjectUserData::cameraGetWorldPosInWindow(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Camera));
        Ogre::Camera* cam = dynamic_cast<Ogre::Camera*>(outObject);
        assert(cam);

        Ogre::Vector3 target;
        SQInteger result = ScriptGetterUtils::vector3Read(vm, &target);
        if(result != 0) return result;

        Ogre::Vector3 hcsPosition = cam->getProjectionMatrix() * cam->getViewMatrix() * target;
        Vector3UserData::vector3ToUserData(vm, hcsPosition);

        return 1;
    }

    SQInteger MovableObjectUserData::cameraSetAspectRatio(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Camera));
        Ogre::Camera* cam = dynamic_cast<Ogre::Camera*>(outObject);
        assert(cam);

        SQFloat aspectRatio;
        sq_getfloat(vm, 2, &aspectRatio);
        if(aspectRatio <= 0) return sq_throwerror(vm, "Camera aspect ratio must be greater than 0.");

        cam->setAspectRatio(aspectRatio);

        return 0;
    }

    SQInteger MovableObjectUserData::cameraGetCameraToViewportRay(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Camera));
        Ogre::Camera* cam = dynamic_cast<Ogre::Camera*>(outObject);
        assert(cam);

        SQFloat x, y;
        sq_getfloat(vm, -1, &y);
        sq_getfloat(vm, -2, &x);

        Ogre::Ray ray = cam->getCameraToViewportRay(x, y);
        RayUserData::RayToUserData(vm, &ray);

        return 1;
    }

    SQInteger MovableObjectUserData::setVisibilityFlags(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Any));

        SQInteger hash;
        sq_getinteger(vm, 2, &hash);
        if(hash < 0) return sq_throwerror(vm, "Hash must be positive.");
        Ogre::uint32 targetHash = static_cast<Ogre::uint32>(hash);

        outObject->setVisibilityFlags(targetHash);

        return 0;
    }

    SQInteger MovableObjectUserData::setRenderQueueGroup(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Any));

        SQInteger renderQueue;
        sq_getinteger(vm, 2, &renderQueue);

        if(renderQueue < 0 || renderQueue > 100) return sq_throwerror(vm, "Render queue must be in range 0-100");
        uint8 target = static_cast<uint8>(renderQueue);

        outObject->setRenderQueueGroup(target);

        return 0;
    }

    SQInteger MovableObjectUserData::setQueryFlags(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Any));

        SQInteger val;
        ASSERT_SQ_RESULT(sq_getinteger(vm, 2, &val));
        Ogre::uint32 targetMask = static_cast<Ogre::uint32>(val);

        outObject->setQueryFlags(targetMask);

        return 0;
    }

    SQInteger MovableObjectUserData::getParentNode(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Any));

        Ogre::SceneNode* node = outObject->getParentSceneNode();
        if(node == 0){
            sq_pushnull(vm);
            return 1;
        }
        SceneNodeUserData::sceneNodeToUserData(vm, node);

        return 1;
    }

    SQInteger MovableObjectUserData::setLightDirection(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Any));
        assert(outObject);
        Ogre::Light* lightObj = static_cast<Ogre::Light*>(outObject);

        Ogre::SceneNode* parentNode = lightObj->getParentSceneNode();
        if(!parentNode){
            return sq_throwerror(vm, "Light must be attached to a node before direction can be set.");
        }

        Ogre::Vector3 outVec;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &outVec));

        lightObj->setDirection(outVec);

        return 0;
    }

    SQInteger MovableObjectUserData::cameraSetDirection(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Camera));
        Ogre::Camera* cam = dynamic_cast<Ogre::Camera*>(outObject);
        assert(cam);

        Ogre::Vector3 target;
        SQInteger result = ScriptGetterUtils::vector3Read(vm, &target);
        if(result != 0) return result;

        cam->setDirection(target);

        return 0;
    }

    SQInteger MovableObjectUserData::cameraGetOrientation(HSQUIRRELVM vm){
        Ogre::MovableObject* outObject = 0;
        SCRIPT_ASSERT_RESULT(readMovableObjectFromUserData(vm, 1, &outObject, MovableObjectType::Camera));
        Ogre::Camera* cam = dynamic_cast<Ogre::Camera*>(outObject);
        assert(cam);

        const Ogre::Quaternion orientation = cam->getOrientation();
        QuaternionUserData::quaternionToUserData(vm, orientation);

        return 1;
    }

    void MovableObjectUserData::setupDelegateTable(HSQUIRRELVM vm){

        //Particle systems are setup in its own class.
        ParticleSystemUserData::setupDelegateTable(vm, &particleSystemDelegateTableObject);

        { //Create item table
            sq_newtable(vm);

            ScriptUtils::addFunction(vm, setDatablock, "setDatablock", 2, ".u|s");
            ScriptUtils::addFunction(vm, setVisibilityFlags, "setVisibilityFlags", 2, ".i");
            ScriptUtils::addFunction(vm, setRenderQueueGroup, "setRenderQueueGroup", 2, ".i");
            ScriptUtils::addFunction(vm, setQueryFlags, "setQueryFlags", 2, ".i");
            ScriptUtils::addFunction(vm, getParentNode, "getParentNode");

            ScriptUtils::addFunction(vm, itemHasSkeleton, "hasSkeleton");
            ScriptUtils::addFunction(vm, itemGetSkeleton, "getSkeleton");
            ScriptUtils::addFunction(vm, itemUseSkeletonInstanceFrom, "useSkeletonInstanceFrom", 2, ".u");

            ScriptUtils::addFunction(vm, getLocalRadius, "getLocalRadius");
            ScriptUtils::addFunction(vm, getLocalAabb, "getLocalAabb");

            sq_resetobject(&itemDelegateTableObject);
            sq_getstackobj(vm, -1, &itemDelegateTableObject);
            sq_addref(vm, &itemDelegateTableObject);
            sq_pop(vm, 1);
        }

        { //Lights
            sq_newtable(vm);

            ScriptUtils::addFunction(vm, setLightType, "setType", 2, ".i");
            ScriptUtils::addFunction(vm, setLightPowerScale, "setPowerScale", 2, ".n");
            //TODO convert to proper colour value
            ScriptUtils::addFunction(vm, setLightDiffuseColour, "setDiffuseColour", 4, ".nnn");
            ScriptUtils::addFunction(vm, setLightSpecularColour, "setSpecularColour", 4, ".nnn");
            ScriptUtils::addFunction(vm, setLightDirection, "setDirection", -2, ".n|unn");
            ScriptUtils::addFunction(vm, setLightAttenuationBasedOnRadius, "setAttenuationBasedOnRadius", 3, ".nn");

            ScriptUtils::addFunction(vm, setVisibilityFlags, "setVisibilityFlags", 2, ".i");
            ScriptUtils::addFunction(vm, setRenderQueueGroup, "setRenderQueueGroup", 2, ".i");
            ScriptUtils::addFunction(vm, setQueryFlags, "setQueryFlags", 2, ".i");
            ScriptUtils::addFunction(vm, getParentNode, "getParentNode");

            ScriptUtils::addFunction(vm, getLocalRadius, "getLocalRadius");
            ScriptUtils::addFunction(vm, getLocalAabb, "getLocalAabb");

            sq_resetobject(&lightDelegateTableObject);
            sq_getstackobj(vm, -1, &lightDelegateTableObject);
            sq_addref(vm, &lightDelegateTableObject);
            sq_pop(vm, 1);
        }

        { //Camera
            sq_newtable(vm);

            ScriptUtils::addFunction(vm, cameraLookAt, "lookAt", -2, ".n|unn");
            ScriptUtils::addFunction(vm, getParentNode, "getParentNode");
            ScriptUtils::addFunction(vm, cameraSetProjectionType, "setProjectionType");
            ScriptUtils::addFunction(vm, cameraSetOrthoWindow, "setOrthoWindow", 3, ".nn");
            ScriptUtils::addFunction(vm, cameraGetWorldPosInWindow, "getWorldPosInWindow", -2, ".n|unn");
            ScriptUtils::addFunction(vm, cameraSetAspectRatio, "setAspectRatio", 2, ".n");
            ScriptUtils::addFunction(vm, cameraGetCameraToViewportRay, "getCameraToViewportRay", 3, ".nn");
            ScriptUtils::addFunction(vm, cameraSetDirection, "setDirection", -2, ".n|unn");
            ScriptUtils::addFunction(vm, cameraGetOrientation, "getOrientation");

            sq_resetobject(&cameraDelegateTableObject);
            sq_getstackobj(vm, -1, &cameraDelegateTableObject);
            sq_addref(vm, &cameraDelegateTableObject);
            sq_pop(vm, 1);
        }

    }

    void MovableObjectUserData::setupConstants(HSQUIRRELVM vm){
        ScriptUtils::declareConstant(vm, "_LIGHT_DIRECTIONAL", (SQInteger)Ogre::Light::LT_DIRECTIONAL);
        ScriptUtils::declareConstant(vm, "_LIGHT_POINT", (SQInteger)Ogre::Light::LT_POINT);
        ScriptUtils::declareConstant(vm, "_LIGHT_SPOTLIGHT", (SQInteger)Ogre::Light::LT_SPOTLIGHT);
        ScriptUtils::declareConstant(vm, "_LIGHT_VPL", (SQInteger)Ogre::Light::LT_VPL);
    }
}
