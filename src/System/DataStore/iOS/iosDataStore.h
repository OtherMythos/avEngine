#pragma once

#ifdef TARGET_APPLE_IPHONE

#include "System/DataStore/DataStore.h"
#include <string>

namespace AV{
    class iosDataStore : public DataStore{
    public:
        bool getInt(const std::string& key, int& outVal) override;
        bool getFloat(const std::string& key, float& outVal) override;
        bool getBool(const std::string& key, bool& outVal) override;
        bool getString(const std::string& key, std::string& outVal) override;

        void setInt(const std::string& key, int val) override;
        void setFloat(const std::string& key, float val) override;
        void setBool(const std::string& key, bool val) override;
        void setString(const std::string& key, const std::string& val) override;

        void remove(const std::string& key) override;
    };
}

#endif //TARGET_APPLE_IPHONE
