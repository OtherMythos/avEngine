#pragma once

#include "ComponentLogic.h"
#include "Nav/NavTypes.h"
#include "OgreVector3.h"

namespace AV{
    class NavigationComponentLogic : public ComponentLogic{
    public:
        static bool add(eId id);
        static bool remove(eId id);

        static bool navigateTo(eId id, const Ogre::Vector3& pos, float targetSpeed);
        static void updatePathFinding(eId id);
    };
}
