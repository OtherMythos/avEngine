#pragma once
#ifdef DEBUG_SERVER
#include <rapidjson/document.h>
#include "System/EnginePrerequisites.h"
#include <string>

namespace AV{
    class AudioManager;

    struct AudioQuery{
        uint64 after = 0, source = 0;
        unsigned int max = 50;
        bool hasAfter = false;
        std::string state, path;
    };

    class AudioInspector{
    public:
        static bool parseUnsigned(const std::string& text, uint64& value);
        static void write(rapidjson::Document& doc, int& status, AudioManager& manager,
                          const std::string& kind, uint64 id, const AudioQuery& query,
                          bool disabled);
    };
}
#endif
