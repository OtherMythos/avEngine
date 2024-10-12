#include "LottieNamespace.h"

#include "rlottie.h"

#include "Scripting/ScriptNamespace/Classes/Lottie/LottieAnimationUserData.h"
#include "Scripting/ScriptNamespace/Classes/Lottie/LottieSurfaceUserData.h"

#include "System/Util/PathUtils.h"
#include "Logger/Log.h"

namespace AV{

    SQInteger LottieNamespace::createAnimation(HSQUIRRELVM vm){
        const SQChar *animationPath;
        sq_getstring(vm, 2, &animationPath);

        std::string outPath;
        formatResToPath(animationPath, outPath);

        if(!fileExists(outPath)){
            return sq_throwerror(vm, (std::string("Animation file at path '") + outPath + "' does not exist.").c_str());
        }

        std::unique_ptr<rlottie::Animation> player = rlottie::Animation::loadFromFile(outPath);
        if(player == nullptr){
            return sq_throwerror(vm, (std::string("Error parsing animation file at path '") + outPath + "'.").c_str());
        }
        std::shared_ptr<rlottie::Animation> sharedPlayer = std::move(player);

        LottieAnimationUserData::lottieAnimationToUserData(vm, sharedPlayer);

        return 1;
    }

    SQInteger LottieNamespace::createSurface(HSQUIRRELVM vm){
        SQInteger width, height;
        sq_getinteger(vm, 2, &width);
        sq_getinteger(vm, 3, &height);

        AV::uint32* bufPtr = static_cast<AV::uint32*>(malloc(width * height * sizeof(AV::uint32)));
        std::shared_ptr<AV::uint32> buffer(bufPtr);
        rlottie::Surface* s = new rlottie::Surface(buffer.get(), width, height, width * sizeof(AV::uint32));

        LottieSurfaceWrapper* wrapped = new LottieSurfaceWrapper{
            std::shared_ptr<rlottie::Surface>(s),
            buffer
        };
        LottieSurfaceUserData::LottieSurfaceToUserData(vm, std::shared_ptr<LottieSurfaceWrapper>(wrapped));

        return 1;
    }

    /**SQNamespace
    @name _lottie
    @desc A namespace to manage lottie animations
    */
    void LottieNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name createAnimation
        @desc Create an instance of a lottie animation.
        @param1:ResPath: ResPath to the lottie animation JSON file which should be used.
        @returns A lottie animation instance.
        */
        ScriptUtils::addFunction(vm, createAnimation, "createAnimation", 2, ".s");
        /**SQFunction
        @name createSurface
        @desc Create an instance of a lottie surface.
        @param1:width: Width of the target surface.
        @param1:height: Height of the target surface.
        @returns A lottie surface
        */
        ScriptUtils::addFunction(vm, createSurface, "createSurface", 3, ".ii");
    }
}
