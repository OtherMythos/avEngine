#ifdef DEBUG_SERVER

#pragma once

#include <rapidjson/document.h>
#include <string>

#include "System/EnginePrerequisites.h"

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
        Wrap a 64-bit integer as a rapidjson value. AV::uint64 is unsigned long long, which
        is not uint64_t on every platform, so passing one straight to AddMember or the Value
        constructor is an ambiguous overload on Linux. SetUint64 is the only unambiguous path.
        */
        rapidjson::Value uint64Value(uint64 value);
        rapidjson::Value int64Value(int64 value);

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
