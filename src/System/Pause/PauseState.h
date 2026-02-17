#pragma once

#include "PauseDataTypes.h"

#include "System/EnginePrerequisites.h"

namespace AV{
    class PauseState{
    private:
        static PauseMask mCurrentMask;
        static uint32 mAnimationPauseMask;
        static uint32 mDefaultAnimationPauseMask;
    public:
        PauseState() = delete;

        static void setMask(PauseMask mask){
            mCurrentMask = mask;
        }

        static PauseMask getMask() { return mCurrentMask; }

        static void setPauseAnimationMask(uint32 mask){
            mAnimationPauseMask = mask;
        }

        static uint32 getPauseAnimationMask() { return mAnimationPauseMask; }

        static void setDefaultAnimationPauseMask(uint32 mask){
            mDefaultAnimationPauseMask = mask;
        }

        static uint32 getDefaultAnimationPauseMask() { return mDefaultAnimationPauseMask; }
    };
}
