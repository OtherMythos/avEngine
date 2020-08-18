#include "SkeletonUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "SkeletonAnimationUserData.h"

#include "BoneUserData.h"
#include "Animation/OgreSkeletonInstance.h"
#include "OgreException.h"

namespace AV{
    SQObject SkeletonUserData::SkeletonDelegateTableObject;

    SkeletonUserData::SkeletonUserData(){

    }

    SkeletonUserData::~SkeletonUserData(){

    }

    void SkeletonUserData::skeletonToUserData(HSQUIRRELVM vm, Ogre::SkeletonInstance* object){
        Ogre::SkeletonInstance** pointer = (Ogre::SkeletonInstance**)sq_newuserdata(vm, sizeof(Ogre::SkeletonInstance*));
        *pointer = object;

        sq_pushobject(vm, SkeletonDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, OgreSkeletonTypeTag);
    }

    UserDataGetResult SkeletonUserData::readSkeletonFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::SkeletonInstance ** outObject){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != OgreSkeletonTypeTag){
            *outObject = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        Ogre::SkeletonInstance** p = (Ogre::SkeletonInstance**)pointer;
        *outObject = *p;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger SkeletonUserData::getAnimation(HSQUIRRELVM vm){
        Ogre::SkeletonInstance* skel;
        SCRIPT_ASSERT_RESULT(readSkeletonFromUserData(vm, 1, &skel));

        SQObjectType t = sq_gettype(vm, 2);

        Ogre::SkeletonAnimation* anim = 0;
        if(t == OT_STRING){
            const SQChar* sqStr;
            sq_getstring(vm, 2, &sqStr);

            try{
                anim = skel->getAnimation(sqStr);
            }catch(Ogre::Exception& e){
                //Not found, return the error.
                SCRIPT_RETURN_OGRE_ERROR("Error getting animation: ", e);
            }
        }else{
            assert(t == OT_INTEGER);
            SQInteger id;
            sq_getinteger(vm, 2, &id);

            const Ogre::SkeletonAnimationVec& anims = skel->getAnimations();
            if(id < 0 || id >= anims.size()) return sq_throwerror(vm, "Requested animation out of range.");

            /*
            Remove the const from the pointer.
            Ogre seems to only provide the vector as a const list, while the get by string functions are non-const.
            I don't like doing const cast but in this case I think the api is missing the option.
            */
            anim = const_cast<Ogre::SkeletonAnimation*>( &(anims[id]) );
        }

        SkeletonAnimationUserData::skeletonAnimationToUserData(vm, anim);

        return 1;
    }

    SQInteger SkeletonUserData::getBone(HSQUIRRELVM vm){
        Ogre::SkeletonInstance* skel;
        SCRIPT_ASSERT_RESULT(readSkeletonFromUserData(vm, 1, &skel));

        SQInteger idx;
        sq_getinteger(vm, 2, &idx);

        Ogre::Bone* bone = skel->getBone(idx);
        BoneUserData::boneToUserData(vm, bone);

        return 1;
    }

    SQInteger SkeletonUserData::getNumBones(HSQUIRRELVM vm){
        Ogre::SkeletonInstance* skel;
        SCRIPT_ASSERT_RESULT(readSkeletonFromUserData(vm, 1, &skel));

        sq_pushinteger(vm, skel->getNumBones());

        return 1;
    }

    SQInteger SkeletonUserData::getNumAnimations(HSQUIRRELVM vm){
        Ogre::SkeletonInstance* skel;
        SCRIPT_ASSERT_RESULT(readSkeletonFromUserData(vm, 1, &skel));

        sq_pushinteger(vm, skel->getAnimations().size());

        return 1;
    }

    void SkeletonUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, getNumAnimations, "getNumAnimations");
        ScriptUtils::addFunction(vm, getAnimation, "getAnimation", 2, ".s|i");
        ScriptUtils::addFunction(vm, getBone, "getBone", 2, ".i");

        ScriptUtils::addFunction(vm, getNumBones, "getNumBones");

        sq_resetobject(&SkeletonDelegateTableObject);
        sq_getstackobj(vm, -1, &SkeletonDelegateTableObject);
        sq_addref(vm, &SkeletonDelegateTableObject);
        sq_pop(vm, 1);
    }
}
