#pragma once

#include <OgreVector3.h>
#include <OgreQuaternion.h>

namespace AV{
    enum class SceneObjectType{
        Child,
        Term,
        Empty,
        Mesh
    };
    struct SceneObjectEntry{
        SceneObjectType type;

        //OPTIMISATION consider moving these bits somewhere else.
        Ogre::Vector3 pos;
        Ogre::Vector3 scale;
        Ogre::Quaternion orientation;
    };
    struct ParsedSceneFile{
        std::vector<SceneObjectEntry> objects;
    };
}
