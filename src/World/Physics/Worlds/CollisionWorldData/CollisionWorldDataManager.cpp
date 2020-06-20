#include "CollisionWorldDataManager.h"

#include <cassert>

namespace AV{
    CollisionWorldDataManager::CollisionWorldDataManager(){

    }

    CollisionWorldDataManager::~CollisionWorldDataManager(){

    }

    uint16 CollisionWorldDataManager::storeData(CollisionObjectType::CollisionObjectType type, const std::string& filePath, const std::string& closureName){
        switch(type){
            case CollisionObjectType::CollisionObjectType::RECEIVER:
                return INVALID_COLLISION_DATA;
                break; //For now
            case CollisionObjectType::CollisionObjectType::SENDER_SCRIPT:
                _insertData({filePath, closureName});
                break;
        }
    }

    uint16 CollisionWorldDataManager::_insertData(const DataEntry& data){
        //TODO Just have it insert for the moment. Later come up with a better solution, i.e it checking it see if that value already exists.
        uint16 id = mData.size();

        mData.push_back(data);

        return id;
    }

    CollisionWorldDataManager::DataEntry* CollisionWorldDataManager::getDataForEntry(uint16 id){
        assert(mData.size() > id);

        return &(mData[id]);
    }
}
