#include "BoneUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "Animation/OgreSkeletonInstance.h"
#include "OgreException.h"

#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/SceneNodeUserData.h"

#include <iostream>
#include "System/EngineFlags.h"

namespace AV{
    SQObject BoneUserData::BoneDelegateTableObject;

    BoneUserData::BoneUserData(){

    }

    BoneUserData::~BoneUserData(){

    }

    void BoneUserData::boneToUserData(HSQUIRRELVM vm, Ogre::Bone* object){
        Ogre::Bone** pointer = (Ogre::Bone**)sq_newuserdata(vm, sizeof(Ogre::Bone*));
        *pointer = object;

        sq_pushobject(vm, BoneDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, OgreBoneTypeTag);
    }

    UserDataGetResult BoneUserData::readBoneFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Bone ** outObject){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != OgreBoneTypeTag){
            *outObject = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        Ogre::Bone** p = (Ogre::Bone**)pointer;
        *outObject = *p;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger BoneUserData::setPosition(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
        Ogre::Bone* bone = 0;
        SCRIPT_ASSERT_RESULT(readBoneFromUserData(vm, 1, &bone));

        Ogre::Vector3 pos;
        SQInteger result = ScriptGetterUtils::vector3Read(vm, &pos);
        if(result != 0) return result;

        bone->setPosition(pos);

        return 0;
    }

    SQInteger BoneUserData::setScale(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
        Ogre::Bone* bone = 0;
        SCRIPT_ASSERT_RESULT(readBoneFromUserData(vm, 1, &bone));

        Ogre::Vector3 target;
        if(!ScriptGetterUtils::vector3Read(vm, &target)){
            return 0;
        }

        bone->setScale(target);

        return 0;
    }

    SQInteger BoneUserData::getName(HSQUIRRELVM vm){
        Ogre::Bone* bone = 0;
        SCRIPT_ASSERT_RESULT(readBoneFromUserData(vm, 1, &bone));

        sq_pushstring(vm, bone->getName().c_str(), -1);

        return 1;
    }

    SQInteger BoneUserData::getNumChildrenBones(HSQUIRRELVM vm){
        Ogre::Bone* bone = 0;
        SCRIPT_ASSERT_RESULT(readBoneFromUserData(vm, 1, &bone));

        //sq_pushinteger(vm, bone->getChildren().size());
        sq_pushinteger(vm, bone->getNumChildren());

        return 1;
    }

    SQInteger BoneUserData::getChildBone(HSQUIRRELVM vm){
        Ogre::Bone* bone = 0;
        SCRIPT_ASSERT_RESULT(readBoneFromUserData(vm, 1, &bone));

        SQInteger idx;
        sq_getinteger(vm, 2, &idx);

        Ogre::Bone* child = bone->getChild(idx);
        boneToUserData(vm, child);

        return 1;
    }

    SQInteger BoneUserData::addTagPoint(HSQUIRRELVM vm){
        Ogre::Bone* bone = 0;
        SCRIPT_ASSERT_RESULT(readBoneFromUserData(vm, 1, &bone));

        Ogre::TagPoint* outTagPoint;
        SCRIPT_CHECK_RESULT(SceneNodeUserData::readTagPointFromUserData(vm, 2, &outTagPoint));

        bone->addTagPoint(outTagPoint);

        return 0;
    }

    void BoneUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", -2, ".n|unn");
        ScriptUtils::addFunction(vm, setScale, "setScale", -2, ".n|unn");

        ScriptUtils::addFunction(vm, getName, "getName");

        ScriptUtils::addFunction(vm, getNumChildrenBones, "getNumChildrenBones");
        ScriptUtils::addFunction(vm, getChildBone, "getChildBone", 2, ".i");

        ScriptUtils::addFunction(vm, addTagPoint, "addTagPoint", 2, ".u");

        sq_resetobject(&BoneDelegateTableObject);
        sq_getstackobj(vm, -1, &BoneDelegateTableObject);
        sq_addref(vm, &BoneDelegateTableObject);
        sq_pop(vm, 1);
    }
}
