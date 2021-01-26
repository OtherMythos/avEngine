#include "AnimationNamespace.h"

#include "System/BaseSingleton.h"
#include "Animation/AnimationManager.h"

#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/SceneNodeUserData.h"
#include "Scripting/ScriptNamespace/Classes/Animation/AnimationInfoUserData.h"

#include "Animation/Script/AnimationScriptParser.h"
#include "System/Util/PathUtils.h"
#include "Logger/Log.h"

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

        AnimationInfoBlockPtr ptr = BaseSingleton::getAnimationManager()->createAnimation(def, blockPtr);
        AnimationInfoUserData::blockPtrToUserData(vm, ptr);

        return 1;
    }

    SQInteger AnimationNamespace::createAnimationInfo(HSQUIRRELVM vm){
        AnimationInfoEntry info[MAX_ANIMATION_INFO];
        memset(&info, 0, sizeof(info));

        SQInteger arraySize = sq_getsize(vm, -1);
        if(arraySize > MAX_ANIMATION_INFO || arraySize <= 0) return sq_throwerror(vm, "Provided entries must contain a max of 16 values.");

        uint8 countIdx = 0;
        sq_pushnull(vm);
        while(SQ_SUCCEEDED(sq_next(vm, -2))){
            SQObjectType t = sq_gettype(vm, -1);
            if(t != OT_USERDATA){
                sq_pop(vm,2);
                continue;
            }

            //In future it will be more than just nodes.
            Ogre::SceneNode* outNode;
            SCRIPT_CHECK_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, -1, &outNode));
            info[countIdx].sceneNode = outNode;

            sq_pop(vm,2);
            countIdx++;
        }
        sq_pop(vm,1);

        AnimationInfoBlockPtr ptr = BaseSingleton::getAnimationManager()->createAnimationInfoBlock(info, 1);
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

        ScriptUtils::addFunction(vm, loadAnimationFile, "loadAnimationFile", 2, ".s");
    }
}
