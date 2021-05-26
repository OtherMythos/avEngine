#pragma once

#include "PauseDataTypes.h"

namespace AV{
    class PauseState{
    private:
        static PauseMask mCurrentMask;
    public:
        PauseState() = delete;

        static void setMask(PauseMask mask){
            mCurrentMask = mask;
        }

        static PauseMask getMask() { return mCurrentMask; }
    };
}
