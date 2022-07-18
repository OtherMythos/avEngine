#pragma once

#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreString.h>

namespace AV{
    enum class SceneObjectType{
        Child,
        Term,
        Empty,
        Mesh
    };
    struct SceneObjectEntry{
        SceneObjectType type;
    };
    struct SceneObjectData{
        union{
            size_t idx;
        };

        Ogre::Vector3 pos;
        Ogre::Vector3 scale;
        Ogre::Quaternion orientation;
    };
    struct ParsedSceneFile{
        //Objects list the types, including marking children and terminators.
        std::vector<SceneObjectEntry> objects;
        //Data for each actual object (no child or terms).
        std::vector<SceneObjectData> data;
        std::vector<Ogre::String> strings;
    };
}
