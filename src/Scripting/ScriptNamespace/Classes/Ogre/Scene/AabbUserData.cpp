#include "AabbUserData.h"

#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptObjectTypeTags.h"
#include "Math/Simple/OgreAabb.h"

namespace AV{
    SQObject AabbUserData::aabbDelegateTableObject;

    void AabbUserData::AabbToUserData(HSQUIRRELVM vm, const Ogre::Aabb* object){
        Ogre::Aabb* pointer = (Ogre::Aabb*)sq_newuserdata(vm, sizeof(Ogre::Aabb));
        *pointer = *object;

        sq_pushobject(vm, aabbDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, OgreAABBTypeTag);
    }

    UserDataGetResult AabbUserData::readAabbFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Aabb* outObject){
        Ogre::Aabb* objPtr;
        UserDataGetResult result = _readAabbPtrFromUserData(vm, stackInx, &objPtr);
        if(result != USER_DATA_GET_SUCCESS) return result;

        *outObject = *objPtr;
        return result;
    }

    UserDataGetResult AabbUserData::_readAabbPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Aabb** outObject){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != OgreAABBTypeTag){
            *outObject = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        Ogre::Aabb* p = static_cast<Ogre::Aabb*>(pointer);
        *outObject = p;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger AabbUserData::getCentre(HSQUIRRELVM vm){
        Ogre::Aabb *boundingBox;
        SCRIPT_ASSERT_RESULT(_readAabbPtrFromUserData(vm, 1, &boundingBox));

        Vector3UserData::vector3ToUserData(vm, boundingBox->mCenter);

        return 1;
    }

    SQInteger AabbUserData::getHalfSize(HSQUIRRELVM vm){
        Ogre::Aabb *boundingBox;
        SCRIPT_ASSERT_RESULT(_readAabbPtrFromUserData(vm, 1, &boundingBox));

        Vector3UserData::vector3ToUserData(vm, boundingBox->mHalfSize);

        return 1;
    }

    SQInteger AabbUserData::getSize(HSQUIRRELVM vm){
        Ogre::Aabb *boundingBox;
        SCRIPT_ASSERT_RESULT(_readAabbPtrFromUserData(vm, 1, &boundingBox));

        Vector3UserData::vector3ToUserData(vm, boundingBox->getSize());

        return 1;
    }

    SQInteger AabbUserData::getMaximum(HSQUIRRELVM vm){
        Ogre::Aabb *boundingBox;
        SCRIPT_ASSERT_RESULT(_readAabbPtrFromUserData(vm, 1, &boundingBox));

        Vector3UserData::vector3ToUserData(vm, boundingBox->getMaximum());

        return 1;
    }

    SQInteger AabbUserData::getMinimum(HSQUIRRELVM vm){
        Ogre::Aabb *boundingBox;
        SCRIPT_ASSERT_RESULT(_readAabbPtrFromUserData(vm, 1, &boundingBox));

        Vector3UserData::vector3ToUserData(vm, boundingBox->getMinimum());

        return 1;
    }

    SQInteger AabbUserData::getRadius(HSQUIRRELVM vm){
        Ogre::Aabb *boundingBox;
        SCRIPT_ASSERT_RESULT(_readAabbPtrFromUserData(vm, 1, &boundingBox));

        sq_pushfloat(vm, boundingBox->getRadius());

        return 1;
    }

    void AabbUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, getCentre, "getCentre");
        ScriptUtils::addFunction(vm, getHalfSize, "getHalfSize");
        ScriptUtils::addFunction(vm, getSize, "getSize");
        ScriptUtils::addFunction(vm, getMaximum, "getMaximum");
        ScriptUtils::addFunction(vm, getMinimum, "getMinimum");
        ScriptUtils::addFunction(vm, getRadius, "getRadius");

        sq_resetobject(&aabbDelegateTableObject);
        sq_getstackobj(vm, -1, &aabbDelegateTableObject);
        sq_addref(vm, &aabbDelegateTableObject);
        sq_pop(vm, 1);

    }
}
