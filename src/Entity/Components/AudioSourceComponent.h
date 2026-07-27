#pragma once

#include "Audio/AudioTypes.h"

namespace AV{
    struct AudioSourceComponent{
        AudioSourceComponent(AudioSourcePtr a, AudioSourcePtr b, bool aPop, bool bPop)
            : aPopulated(aPop), bPopulated(bPop), objA(a), objB(b) { }
        ~AudioSourceComponent(){
            objA.reset();
            objB.reset();
        }

        bool aPopulated;
        bool bPopulated;
        AudioSourcePtr objA;
        AudioSourcePtr objB;

    };
}
