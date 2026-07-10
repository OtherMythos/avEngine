#ifdef DEBUG_SERVER

#pragma once

#include <rapidjson/document.h>

namespace AV{
    /**
    Serialises a cheap liveness summary of the engine into a JSON document.

    Reads only main-thread-safe singletons, so it must be run through the MainThreadQueue.
    */
    class StatusInspector{
    public:
        /**
        @param doc The document to populate (set as an object).
        @param uptimeSeconds How long the debug server has been running.
        */
        static void writeStatus(rapidjson::Document& doc, double uptimeSeconds);
    };
}

#endif
