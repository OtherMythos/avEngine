#ifdef DEBUG_SERVER

#include "gtest/gtest.h"

#include "System/DebugServer/Inspection/DebugJsonUtil.h"

#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <Math/Simple/OgreAabb.h>

#include <rapidjson/document.h>

using namespace AV;

TEST(DebugJsonUtilTests, vector3SerialisesAsXyzArray){
    rapidjson::Document doc;
    rapidjson::Value v = DebugJsonUtil::vector3(Ogre::Vector3(1.0f, 2.0f, 3.0f), doc.GetAllocator());

    ASSERT_TRUE(v.IsArray());
    ASSERT_EQ(v.Size(), 3u);
    ASSERT_FLOAT_EQ(v[0].GetFloat(), 1.0f);
    ASSERT_FLOAT_EQ(v[1].GetFloat(), 2.0f);
    ASSERT_FLOAT_EQ(v[2].GetFloat(), 3.0f);
}

TEST(DebugJsonUtilTests, quaternionSerialisesAsWxyzArray){
    rapidjson::Document doc;
    rapidjson::Value v = DebugJsonUtil::quaternion(Ogre::Quaternion(0.1f, 0.2f, 0.3f, 0.4f), doc.GetAllocator());

    ASSERT_TRUE(v.IsArray());
    ASSERT_EQ(v.Size(), 4u);
    //Ogre::Quaternion(w, x, y, z)
    ASSERT_FLOAT_EQ(v[0].GetFloat(), 0.1f);
    ASSERT_FLOAT_EQ(v[1].GetFloat(), 0.2f);
    ASSERT_FLOAT_EQ(v[2].GetFloat(), 0.3f);
    ASSERT_FLOAT_EQ(v[3].GetFloat(), 0.4f);
}

TEST(DebugJsonUtilTests, aabbSerialisesCentreAndHalfSize){
    rapidjson::Document doc;
    Ogre::Aabb aabb(Ogre::Vector3(1.0f, 2.0f, 3.0f), Ogre::Vector3(0.5f, 0.5f, 0.5f));
    rapidjson::Value v = DebugJsonUtil::aabb(aabb, doc.GetAllocator());

    ASSERT_TRUE(v.IsObject());
    ASSERT_TRUE(v.HasMember("centre"));
    ASSERT_TRUE(v.HasMember("halfSize"));
    ASSERT_FLOAT_EQ(v["centre"][0].GetFloat(), 1.0f);
    ASSERT_FLOAT_EQ(v["halfSize"][0].GetFloat(), 0.5f);
}

TEST(DebugJsonUtilTests, errorBodyProducesErrorObject){
    std::string body = DebugJsonUtil::errorBody("something broke");

    rapidjson::Document doc;
    doc.Parse(body.c_str());
    ASSERT_FALSE(doc.HasParseError());
    ASSERT_TRUE(doc.IsObject());
    ASSERT_TRUE(doc.HasMember("error"));
    ASSERT_STREQ(doc["error"].GetString(), "something broke");
}

#endif
