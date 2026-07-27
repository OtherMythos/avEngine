#pragma once

#include "ComponentLogic.h"

#include "OgreVector3.h"
#include "Mesh/OgreMeshManager.h"
#include "OgreString.h"

namespace entityx{
    class Entity;
}

namespace AV{

    class OgreMeshComponentLogic : public ComponentLogic{
    public:
        static void add(eId id, const Ogre::String &mesh);
        static void add(eId id, OgreMeshManager::OgreMeshPtr mesh);
        static bool remove(eId id);

        static void reposition(eId id);
        static void repositionKnown(eId id, const Ogre::Vector3& pos);

        static void orientate(eId id, Ogre::Quaternion orientation);

        static OgreMeshManager::OgreMeshPtr getMesh(eId id);

    private:
        static void _add(entityx::Entity& entity, OgreMeshManager::OgreMeshPtr mesh);
    };
}
