#ifdef DEBUG_SERVER

#include "DebugJsonUtil.h"

#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <Math/Simple/OgreAabb.h>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "Logger/Log.h"

namespace AV{
    namespace DebugJsonUtil{
        rapidjson::Value vector3(const Ogre::Vector3& vec, rapidjson::Document::AllocatorType& allocator){
            rapidjson::Value arr(rapidjson::kArrayType);
            arr.PushBack(vec.x, allocator);
            arr.PushBack(vec.y, allocator);
            arr.PushBack(vec.z, allocator);
            return arr;
        }

        rapidjson::Value quaternion(const Ogre::Quaternion& quat, rapidjson::Document::AllocatorType& allocator){
            rapidjson::Value arr(rapidjson::kArrayType);
            arr.PushBack(quat.w, allocator);
            arr.PushBack(quat.x, allocator);
            arr.PushBack(quat.y, allocator);
            arr.PushBack(quat.z, allocator);
            return arr;
        }

        rapidjson::Value aabb(const Ogre::Aabb& aabb, rapidjson::Document::AllocatorType& allocator){
            rapidjson::Value obj(rapidjson::kObjectType);
            obj.AddMember("centre", vector3(aabb.mCenter, allocator), allocator);
            obj.AddMember("halfSize", vector3(aabb.mHalfSize, allocator), allocator);
            return obj;
        }

        std::string toString(const rapidjson::Document& doc){
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<
                rapidjson::StringBuffer,
                rapidjson::UTF8<>,
                rapidjson::UTF8<>,
                rapidjson::CrtAllocator,
                rapidjson::kWriteNanAndInfFlag
            > writer(buffer);
            if(!doc.Accept(writer)){
                AV_ERROR("DebugJsonUtil::toString: rapidjson Writer rejected the document — JSON output may be incomplete.");
            }
            return std::string(buffer.GetString(), buffer.GetSize());
        }

        std::string errorBody(const std::string& message){
            rapidjson::Document doc;
            rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
            doc.SetObject();
            rapidjson::Value msg(message.c_str(), allocator);
            doc.AddMember("error", msg, allocator);
            return toString(doc);
        }
    }
}

#endif
