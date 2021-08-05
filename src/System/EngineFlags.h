#pragma once

namespace AV{

    #define CHECK_SCENE_CLEAN() \
        if(EngineFlags::sceneClean()) return sq_throwerror(vm, "Call invalid while the scene is guaranteed clean.");

    /**
    A class to manage engine flags which change dynamically during engine runtime.
    For instance, keeping track of whether the engine scene is currently considered clean.
    This flag is updated each frame.

    */
    class EngineFlags{
    public:
        EngineFlags() = delete;
        ~EngineFlags() = delete;

    private:
        static bool mSceneClean;

    public:
        static bool sceneClean() { return mSceneClean; }
    };
}
