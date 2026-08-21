#pragma once

#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreString.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include "System/EnginePrerequisites.h"

namespace AV{
    enum class SceneObjectType{
        Child,
        Term,
        Empty,
        Mesh,

        User0,
        User1,
        User2,
        User3,
        User4,
        User5,
        User6
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
        //Index into strings of this object's tag, or -1 if it wasn't given one.
        int tag;
        //Index into data of this object's parent, or -1 if this object sits at the root.
        int parent;
        //The type of this object, mirroring its entry in the objects list.
        SceneObjectType type;
    };
    struct ParsedSceneFile{
        //Objects list the types, including marking children and terminators.
        std::vector<SceneObjectEntry> objects;
        //Data for each actual object (no child or terms).
        std::vector<SceneObjectData> data;
        std::vector<Ogre::String> strings;

        //Parallel to data. The indices into data of each object's children.
        //Populated as the file is parsed so a lookup never has to walk the objects list.
        std::vector<std::vector<uint32>> childIndices;
        //Indices into data of the objects whose parent is -1.
        std::vector<uint32> rootIndices;

        //Tag to the index into data of the object carrying it. Tags are unique within a scene,
        //so this is the whole lookup, no walking involved.
        std::unordered_map<Ogre::String, uint32> tags;
    };

    typedef std::shared_ptr<ParsedSceneFile> ParsedSceneFilePtr;
}
