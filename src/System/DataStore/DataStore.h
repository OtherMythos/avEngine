#pragma once

#include <string>

namespace AV{
    class DataStore{
    public:
        virtual ~DataStore() = default;

        virtual bool getInt(const std::string& key, int& outVal) = 0;
        virtual bool getFloat(const std::string& key, float& outVal) = 0;
        virtual bool getBool(const std::string& key, bool& outVal) = 0;
        virtual bool getString(const std::string& key, std::string& outVal) = 0;

        virtual void setInt(const std::string& key, int val) = 0;
        virtual void setFloat(const std::string& key, float val) = 0;
        virtual void setBool(const std::string& key, bool val) = 0;
        virtual void setString(const std::string& key, const std::string& val) = 0;

        virtual void remove(const std::string& key) = 0;

        static void setInstance(DataStore* store);
        static DataStore* getInstance();

    private:
        static DataStore* mInstance;
    };
}
