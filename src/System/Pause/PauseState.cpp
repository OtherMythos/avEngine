#include "PauseState.h"

namespace AV{
    PauseMask PauseState::mCurrentMask = 0u;
    uint32 PauseState::mAnimationPauseMask = 0u;
    uint32 PauseState::mDefaultAnimationPauseMask = 0u;
}
