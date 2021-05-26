#include "AnimationNamespace.h"

#include "System/BaseSingleton.h"
#include "Animation/AnimationManager.h"

#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/SceneNodeUserData.h"
#include "Scripting/ScriptNamespace/Classes/Animation/AnimationInfoUserData.h"
#include "Scripting/ScriptNamespace/Classes/Animation/AnimationInstanceUserData.h"

#include "Scripting/ScriptNamespace/Classes/Ogre/Hlms/DatablockUserData.h"
#include "Scripting/ScriptObjectTypeTags.h"
#include "Animation/Script/AnimationScriptParser.h"
#include "System/Util/PathUtils.h"
#include "Logger/Log.h"

#include "OgreHlmsPbsDatablock.h"

namespace AV{

    class AnimationScriptLogger : public AnimationScriptParserLogger{
        virtual void notifyError(const std::string& message){
            AV_ERROR(message);
        }

        virtual void notifyWarning(const std::string& message){
            AV_WARN(message);
        }
    };
    static AnimationScriptLogger mLogger;

    SQInteger AnimationNamespace::createAnimation(HSQUIRRELVM vm){
        const SQChar *animationName;
        sq_getstring(vm, 2, &animationName);

        SequenceAnimationDefPtr def = BaseSingleton::getAnimationManager()->getAnimationDefinition(animationName);
        if(!def){
            const std::string outVal("Could not find an animation def with the name '" + std::string(animationName) + "'");
            return sq_throwerror(vm, outVal.c_str());
        }

        AnimationInfoBlockPtr blockPtr;
        AnimationInfoUserData::readBlockPtrFromUserData(vm, 3, &blockPtr);

        bool success = true;
        SequenceAnimationPtr ptr = BaseSingleton::getAnimationManager()->createAnimation(def, blockPtr, success);
        if(!success){
            return sq_throwerror(vm, "Error creating animation. Provided animation data does not match definition.");
        }
        AnimationInstanceUserData::animationPtrToUserData(vm, ptr);

        return 1;
    }

    SQInteger AnimationNamespace::createAnimationInfo(HSQUIRRELVM vm){
        AnimationInfoEntry info[MAX_ANIMATION_INFO];
        memset(&info, 0, sizeof(info));

        SQInteger arraySize = sq_getsize(vm, -1);
        if(arraySize > MAX_ANIMATION_INFO || arraySize <= 0) return sq_throwerror(vm, "Provided entries must contain a max of 16 values.");

        AnimationInfoTypeHash hash = 0;
        uint8 countIdx = 0;
        sq_pushnull(vm);
        while(SQ_SUCCEEDED(sq_next(vm, -2))){
            if(countIdx >= 16){
                sq_pop(vm,2);
                return sq_throwerror(vm, "More than 16 values were supplied when creating animation info.");
            }
            SQObjectType t = sq_gettype(vm, -1);
            if(t != OT_USERDATA){
                sq_pop(vm,2);
                continue;
            }
            SQUserPointer type;
            if(SQ_FAILED(sq_gettypetag(vm, -1, &type))){
                sq_pop(vm,2);
                continue;
            }

            if(type == SceneNodeTypeTag){
                Ogre::SceneNode* outNode;
                SCRIPT_ASSERT_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, -1, &outNode));
                info[countIdx].sceneNode = outNode;
                hash |= ANIM_INFO_SCENE_NODE << MAX_ANIMATION_INFO_BITS*countIdx;
            }else if(type == datablockTypeTag){
                Ogre::HlmsDatablock* outBlock;
                SCRIPT_ASSERT_RESULT(DatablockUserData::getPtrFromUserData(vm, -1, &outBlock));
                Ogre::HlmsPbsDatablock* pbsBlock = dynamic_cast<Ogre::HlmsPbsDatablock*>(outBlock);
                assert(pbsBlock);
                info[countIdx].pbsDatablock = pbsBlock;
                hash |= ANIM_INFO_PBS_DATABLOCK << MAX_ANIMATION_INFO_BITS*countIdx;
            }else{
                //Throw an error.
                SCRIPT_CHECK_RESULT(USER_DATA_GET_INCORRECT_TYPE);
            }


            sq_pop(vm,2);
            countIdx++;
        }
        sq_pop(vm,1);

        AnimationInfoBlockPtr ptr = BaseSingleton::getAnimationManager()->createAnimationInfoBlock(info, countIdx, hash);
        AnimationInfoUserData::blockPtrToUserData(vm, ptr);

        return 1;
    }

    SQInteger AnimationNamespace::loadAnimationFile(HSQUIRRELVM vm){
        const SQChar *animationPath;
        sq_getstring(vm, 2, &animationPath);

        std::string outString;
        formatResToPath(animationPath, outString);

        AnimationScriptParser parser;
        AnimationParserOutput output;
        if(!parser.parseFile(outString, output, &mLogger)){
            return sq_throwerror(vm, "Unable to parse the provided animation file.");
        }
        BaseSingleton::getAnimationManager()->addAnimationDefinitionsFromParser(output);

        return 0;
    }

    SQInteger AnimationNamespace::getNumActiveAnimations(HSQUIRRELVM vm){
        size_t num = BaseSingleton::getAnimationManager()->getNumActiveAnimations();
        sq_pushinteger(vm, static_cast<SQInteger>(num));

        return 1;
    }

    /**SQNamespace
    @name _animation
    @desc A namespace to control animations and sequence animations.
    */
    void AnimationNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name createAnimation
        @desc Create an instance of an animation from a definition.
        @param1:String:The name of the animation definition from which this one should be created.
        @param2:AnimationInfo:Information the animation needs to complete. Created by createAnimationInfo().
        @returns An instance of an animation created from the specified animation definiton.
        */
        ScriptUtils::addFunction(vm, createAnimation, "createAnimation", 3, ".su");
        /**SQFunction
        @name createAnimationInfo
        @desc Create an animation info object from a list of objects. This info is used to construct an animation.
        @param1:array:An array with max size 16 containing animatable objects.
        @returns An animation info instance.
        */
        ScriptUtils::addFunction(vm, createAnimationInfo, "createAnimationInfo", 2, ".a");
        /**SQFunction
        @name loadAnimationFile
        @desc Load and parse animations from an animation file
        @param1:String:A res path to an animation file.
        */
        ScriptUtils::addFunction(vm, loadAnimationFile, "loadAnimationFile", 2, ".s");
        /**SQFunction
        @name getNumActiveAnimations
        @desc Get the number of active animations currently run by the animation manager.
        @returns An integer of the number of animations.
        */
        ScriptUtils::addFunction(vm, getNumActiveAnimations, "getNumActiveAnimations");
    }
}
