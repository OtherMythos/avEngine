#pragma once

#include "ComponentLogic.h"

#include "OgreVector3.h"

namespace AV{
    /**
     A class to contain logic for components that all entities possess.
     */
    class FundamentalLogic : public ComponentLogic{
    public:
        static Ogre::Vector3 getPosition(eId id);

        /**
         Set whether or not the entity is tracked.
         This should only be called by
         */


    };
}
