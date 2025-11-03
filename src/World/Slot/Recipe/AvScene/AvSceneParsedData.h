#pragma once

#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreString.h>

#include "System/EnginePrerequisites.h"

namespace AV{
    enum class SceneObjectType{
        Child,
        Term,
        Empty,
        Mesh,

        User0,
        User1,
        User2
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
        uint8 animIdx;
        int name;
    };
    struct ParsedSceneFile{
        //Objects list the types, including marking children and terminators.
        std::vector<SceneObjectEntry> objects;
        //Data for each actual object (no child or terms).
        std::vector<SceneObjectData> data;
        std::vector<Ogre::String> strings;
    };
}
