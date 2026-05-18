#pragma once

#include "DataStore.h"

namespace AV{
    class DataStoreNull : public DataStore{
    public:
        bool getInt(const std::string& key, int& outVal) override{ return false; }
        bool getFloat(const std::string& key, float& outVal) override{ return false; }
        bool getBool(const std::string& key, bool& outVal) override{ return false; }
        bool getString(const std::string& key, std::string& outVal) override{ return false; }

        void setInt(const std::string& key, int val) override{}
        void setFloat(const std::string& key, float val) override{}
        void setBool(const std::string& key, bool val) override{}
        void setString(const std::string& key, const std::string& val) override{}

        void remove(const std::string& key) override{}
    };
}
