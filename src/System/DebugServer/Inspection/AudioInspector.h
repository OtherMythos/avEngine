#pragma once
#ifdef DEBUG_SERVER
#include <rapidjson/document.h>
#include <cstdint>
#include <string>

namespace AV{
    class AudioManager;

    struct AudioQuery{
        uint64_t after = 0, source = 0;
        unsigned int max = 50;
        bool hasAfter = false;
        std::string state, path;
    };

    class AudioInspector{
    public:
        static bool parseUnsigned(const std::string& text, uint64_t& value);
        static void write(rapidjson::Document& doc, int& status, AudioManager& manager,
                          const std::string& kind, uint64_t id, const AudioQuery& query,
                          bool disabled);
    };
}
#endif
