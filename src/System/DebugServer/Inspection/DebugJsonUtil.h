#ifdef DEBUG_SERVER

#pragma once

#include <rapidjson/document.h>
#include <string>

namespace Ogre{
    class Vector3;
    class Quaternion;
    class Aabb;
}

namespace AV{
    /**
    Helpers for serialising engine value types into rapidjson values.

    Each helper returns a rapidjson::Value bound to the supplied allocator, ready to
    be added to a document or a parent value.
    */
    namespace DebugJsonUtil{
        /** [x, y, z] */
        rapidjson::Value vector3(const Ogre::Vector3& vec, rapidjson::Document::AllocatorType& allocator);
        /** [w, x, y, z] */
        rapidjson::Value quaternion(const Ogre::Quaternion& quat, rapidjson::Document::AllocatorType& allocator);
        /** { "centre": [x,y,z], "halfSize": [x,y,z] } */
        rapidjson::Value aabb(const Ogre::Aabb& aabb, rapidjson::Document::AllocatorType& allocator);

        /**
        Serialise a rapidjson document to a compact JSON string.
        */
        std::string toString(const rapidjson::Document& doc);

        /**
        Build an error document body: { "error": "message" }.
        */
        std::string errorBody(const std::string& message);
    }
}

#endif
