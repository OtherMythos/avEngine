#pragma once

#include "World/Physics/Worlds/CollisionWorldUtils.h"
#include "System/EnginePrerequisites.h"

#include <string>
#include <vector>

namespace AV{

    static const uint16 INVALID_COLLISION_DATA = 0;

    /**
    A class responsible for managing internal data associated with collision objects.

    Each collision object can have some data attached to it, for example, scripts and sounds.
    The engine has to store internally what sort of data needs to be executed.
    This class is responsible for storing types of data between the different collision object types (receiver, script sender, etc)
    */
    class CollisionWorldDataManager{
    public:
        CollisionWorldDataManager();
        ~CollisionWorldDataManager();

    public:
        //Data for collision objects.
        struct DataEntry{
            std::string filePath;
            std::string closureName;
            //These scripts have an expected parameter callback, so no sanity checks on the format are necessary.
        };

        /**
        Get the data for a specific entry.

        The returned pointer points into a vector, so should be dealt with without creating any other data.
        */
        DataEntry* getDataForEntry(uint16 id);

        uint16 storeData(CollisionObjectType::CollisionObjectType type, const std::string& filePath, const std::string& closureName);

    private:
        std::vector<DataEntry> mData;

        uint16 _insertData(const DataEntry& data);
    };
}
