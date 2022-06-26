#pragma once

#include <string>

namespace AV{

    #define CHECK_SCENE_CLEAN() \
        if(EngineFlags::sceneClean()) return sq_throwerror(vm, "Call is invalid as the scene is currently guaranteed clean.");

    /**
    A class to manage engine flags which change dynamically during engine runtime.
    For instance, keeping track of whether the engine scene is currently considered clean.
    This flag is updated each frame.

    */
    class EngineFlags{
    public:
        EngineFlags() = delete;
        ~EngineFlags() = delete;

        static const std::string ENGINE_RES_PREREQUISITE;

    private:
        static bool mSceneClean;

    public:
        static bool sceneClean() { return mSceneClean; }

        //Internal function. Only set when the safeScene function is about to be called.
        static void _setSceneClear(bool clear) { mSceneClean = clear; }

        /**
        Checks whether a resource group name is valid, i.e it doesn't clash with any engine reserved keywords.
        @returns true or false depending on validity.
        */
        static bool resourceGroupValid(const std::string& resGroupName);
    };
}
