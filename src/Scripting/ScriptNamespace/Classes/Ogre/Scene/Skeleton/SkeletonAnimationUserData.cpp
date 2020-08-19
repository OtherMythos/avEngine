#include "SkeletonAnimationUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "Animation/OgreSkeletonInstance.h"
#include "Animation/OgreSkeletonAnimationDef.h"
#include "OgreException.h"

#include <iostream>

namespace AV{
    SQObject SkeletonAnimationUserData::SkeletonAnimationDelegateTableObject;

    SkeletonAnimationUserData::SkeletonAnimationUserData(){

    }

    SkeletonAnimationUserData::~SkeletonAnimationUserData(){

    }

    void SkeletonAnimationUserData::skeletonAnimationToUserData(HSQUIRRELVM vm, Ogre::SkeletonAnimation* object){
        Ogre::SkeletonAnimation** pointer = (Ogre::SkeletonAnimation**)sq_newuserdata(vm, sizeof(Ogre::SkeletonAnimation*));
        *pointer = object;

        sq_pushobject(vm, SkeletonAnimationDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, OgreSkeletonAnimationTypeTag);
    }

    UserDataGetResult SkeletonAnimationUserData::readSkeletonAnimationFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::SkeletonAnimation ** outObject){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != OgreSkeletonAnimationTypeTag){
            *outObject = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        Ogre::SkeletonAnimation** p = (Ogre::SkeletonAnimation**)pointer;
        *outObject = *p;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger SkeletonAnimationUserData::addTime(HSQUIRRELVM vm){
        Ogre::SkeletonAnimation* anim = 0;
        SCRIPT_ASSERT_RESULT(readSkeletonAnimationFromUserData(vm, 1, &anim));

        SQFloat timeVal;
        sq_getfloat(vm, 2, &timeVal);

        anim->addTime(timeVal);

        return 0;
    }

    SQInteger SkeletonAnimationUserData::setTime(HSQUIRRELVM vm){
        Ogre::SkeletonAnimation* anim = 0;
        SCRIPT_ASSERT_RESULT(readSkeletonAnimationFromUserData(vm, 1, &anim));

        SQFloat timeVal;
        sq_getfloat(vm, 2, &timeVal);

        anim->setTime(timeVal);

        return 0;
    }

    SQInteger SkeletonAnimationUserData::setLoop(HSQUIRRELVM vm){
        Ogre::SkeletonAnimation* anim = 0;
        SCRIPT_ASSERT_RESULT(readSkeletonAnimationFromUserData(vm, 1, &anim));

        SQBool loop;
        sq_getbool(vm, 2, &loop);

        anim->setLoop(loop);

        return 0;
    }

    SQInteger SkeletonAnimationUserData::setEnabled(HSQUIRRELVM vm){
        Ogre::SkeletonAnimation* anim = 0;
        SCRIPT_ASSERT_RESULT(readSkeletonAnimationFromUserData(vm, 1, &anim));

        SQBool enabled;
        sq_getbool(vm, 2, &enabled);

        anim->setEnabled(enabled);

        return 0;
    }

    SQInteger SkeletonAnimationUserData::getName(HSQUIRRELVM vm){
        Ogre::SkeletonAnimation* anim = 0;
        SCRIPT_ASSERT_RESULT(readSkeletonAnimationFromUserData(vm, 1, &anim));

        const Ogre::String& name = anim->getDefinition()->getNameStr();

        sq_pushstring(vm, name.c_str(), -1);

        return 1;
    }

    SQInteger SkeletonAnimationUserData::getCurrentTime(HSQUIRRELVM vm){
        Ogre::SkeletonAnimation* anim = 0;
        SCRIPT_ASSERT_RESULT(readSkeletonAnimationFromUserData(vm, 1, &anim));

        sq_pushfloat(vm, anim->getCurrentTime());

        return 1;
    }

    SQInteger SkeletonAnimationUserData::getCurrentFrame(HSQUIRRELVM vm){
        Ogre::SkeletonAnimation* anim = 0;
        SCRIPT_ASSERT_RESULT(readSkeletonAnimationFromUserData(vm, 1, &anim));

        sq_pushfloat(vm, anim->getCurrentFrame());

        return 1;
    }

    SQInteger SkeletonAnimationUserData::setWeight(HSQUIRRELVM vm){
        Ogre::SkeletonAnimation* anim = 0;
        SCRIPT_ASSERT_RESULT(readSkeletonAnimationFromUserData(vm, 1, &anim));

        SQFloat w;
        sq_getfloat(vm, 2, &w);

        anim->mWeight = w;

        return 0;
    }

    SQInteger SkeletonAnimationUserData::getWeight(HSQUIRRELVM vm){
        Ogre::SkeletonAnimation* anim = 0;
        SCRIPT_ASSERT_RESULT(readSkeletonAnimationFromUserData(vm, 1, &anim));

        sq_pushfloat(vm, anim->mWeight);

        return 1;
    }

    void SkeletonAnimationUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, addTime, "addTime", 2, ".n");
        ScriptUtils::addFunction(vm, setTime, "setTime", 2, ".n");
        ScriptUtils::addFunction(vm, setLoop, "setLoop", 2, ".b");
        ScriptUtils::addFunction(vm, setEnabled, "setEnabled", 2, ".b");
        ScriptUtils::addFunction(vm, getName, "getName");
        ScriptUtils::addFunction(vm, getCurrentTime, "getCurrentTime");
        ScriptUtils::addFunction(vm, getCurrentFrame, "getCurrentFrame");

        ScriptUtils::addFunction(vm, setWeight, "setWeight", 2, ".f");
        ScriptUtils::addFunction(vm, getWeight, "getWeight");

        sq_resetobject(&SkeletonAnimationDelegateTableObject);
        sq_getstackobj(vm, -1, &SkeletonAnimationDelegateTableObject);
        sq_addref(vm, &SkeletonAnimationDelegateTableObject);
        sq_pop(vm, 1);
    }
}
