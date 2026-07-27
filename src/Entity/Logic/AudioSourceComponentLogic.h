#pragma once

#include "ComponentLogic.h"
#include "System/EnginePrerequisites.h"
#include "Audio/AudioTypes.h"

#include "OgreVector3.h"

namespace AV{
    class AudioSourceComponentLogic : public ComponentLogic{
    public:
        static bool add(eId id, AudioSourcePtr a, AudioSourcePtr b, bool aPopulated, bool bPopulated);
        static bool remove(eId id);

        static AudioSourcePtr getAudioSource(eId id, uint8 animId);
        static bool setAudioSource(eId id, uint8 animId, AudioSourcePtr ptr);

        static void repositionKnown(eId id, const Ogre::Vector3& pos);
    };
}
