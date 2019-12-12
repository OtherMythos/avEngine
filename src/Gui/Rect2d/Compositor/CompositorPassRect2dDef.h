#pragma once

#include "Compositor/Pass/OgreCompositorPassDef.h"

namespace AV{
    class CompositorPassRect2dDef : public Ogre::CompositorPassDef{
    public:
        CompositorPassRect2dDef(Ogre::CompositorTargetDef *parentTargetDef)
            : Ogre::CompositorPassDef(Ogre::PASS_CUSTOM, parentTargetDef){
            mProfilingId = "Rect2dCompositor";
        }
    };
}
