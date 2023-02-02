#include "RayUserData.h"

#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"
#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/Classes/PlaneUserData.h"
#include "OgreRay.h"

namespace AV{
    SQObject RayUserData::rayDelegateTableObject;

    void RayUserData::RayToUserData(HSQUIRRELVM vm, const Ogre::Ray* object){
        Ogre::Ray* pointer = (Ogre::Ray*)sq_newuserdata(vm, sizeof(Ogre::Ray));
        *pointer = *object;

        sq_pushobject(vm, rayDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, RayTypeTag);
    }

    UserDataGetResult RayUserData::readRayFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Ray* outObject){
        Ogre::Ray* objPtr;
        UserDataGetResult result = _readRayPtrFromUserData(vm, stackInx, &objPtr);
        if(result != USER_DATA_GET_SUCCESS) return result;

        *outObject = *objPtr;
        return result;
    }

    UserDataGetResult RayUserData::_readRayPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Ray** outObject){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != RayTypeTag){
            *outObject = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        Ogre::Ray* p = static_cast<Ogre::Ray*>(pointer);
        *outObject = p;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger RayUserData::getDirection(HSQUIRRELVM vm){
        Ogre::Ray *ray;
        SCRIPT_ASSERT_RESULT(_readRayPtrFromUserData(vm, 1, &ray));

        Vector3UserData::vector3ToUserData(vm, ray->getDirection());

        return 1;
    }

    SQInteger RayUserData::getOrigin(HSQUIRRELVM vm){
        Ogre::Ray *ray;
        SCRIPT_ASSERT_RESULT(_readRayPtrFromUserData(vm, 1, &ray));

        Vector3UserData::vector3ToUserData(vm, ray->getOrigin());

        return 1;
    }

    SQInteger RayUserData::getPoint(HSQUIRRELVM vm){
        Ogre::Ray *ray;
        SCRIPT_ASSERT_RESULT(_readRayPtrFromUserData(vm, 1, &ray));

        SQFloat distance;
        sq_getfloat(vm, 2, &distance);

        Vector3UserData::vector3ToUserData(vm, ray->getPoint(distance));

        return 1;
    }

    SQInteger RayUserData::intersects(HSQUIRRELVM vm){
        Ogre::Ray *ray;
        SCRIPT_ASSERT_RESULT(_readRayPtrFromUserData(vm, 1, &ray));

        Ogre::Plane p;
        SCRIPT_CHECK_RESULT(PlaneUserData::readPlaneFromUserData(vm, 2, &p));

        auto result = ray->intersects(p);
        if(!result.first){
            sq_pushbool(vm, result.first);
            return 1;
        }
        sq_pushfloat(vm, result.second);

        return 1;
    }

    SQInteger RayUserData::createRay(HSQUIRRELVM vm){
        SQInteger top = sq_gettop(vm);

        Ogre::Vector3 pos(Ogre::Vector3::ZERO);
        Ogre::Vector3 direction(Ogre::Vector3::ZERO);
        if(top == 3){
            UserDataGetResult result = Vector3UserData::readVector3FromUserData(vm, 2, &pos);
            if(result != USER_DATA_GET_SUCCESS){
                //Check for a slotPosition
                SlotPosition slotPos;
                result = SlotPositionClass::getSlotFromInstance(vm, 2, &slotPos);
                SCRIPT_CHECK_RESULT(result);
                pos = slotPos.toOgre();
            }
            SCRIPT_CHECK_RESULT(Vector3UserData::readVector3FromUserData(vm, 3, &direction));
        }

        Ogre::Ray targetRay(pos, direction);
        RayToUserData(vm, &targetRay);

        return 1;
    }

    void RayUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, getDirection, "getDirection");
        ScriptUtils::addFunction(vm, getOrigin, "getOrigin");
        ScriptUtils::addFunction(vm, getPoint, "getPoint", 2, "un");
        ScriptUtils::addFunction(vm, intersects, "intersects", 2, "uu");

        sq_resetobject(&rayDelegateTableObject);
        sq_getstackobj(vm, -1, &rayDelegateTableObject);
        sq_addref(vm, &rayDelegateTableObject);
        sq_pop(vm, 1);

        {
            sq_pushroottable(vm);
            ScriptUtils::addFunction(vm, createRay, "Ray", -1, ".uu");
            sq_pop(vm, 1);
        }

    }
}
