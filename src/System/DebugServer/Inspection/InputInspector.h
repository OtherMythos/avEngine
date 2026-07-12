#ifdef DEBUG_SERVER

#pragma once

#include <rapidjson/document.h>

namespace AV{
    class InputPlayback;

    /**
    Serialises the input action sets (discovery) and the currently spoofed input state.
    Runs on the main thread via the MainThreadQueue.
    */
    class InputInspector{
    public:
        //The action sets the project defines, so an agent can learn valid action names.
        static void writeActions(rapidjson::Document& doc, int& status);

        //What the debug server is currently spoofing.
        static void writeState(rapidjson::Document& doc, const InputPlayback& playback);
    };
}

#endif
