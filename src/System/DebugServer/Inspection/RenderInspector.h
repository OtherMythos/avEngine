#ifdef DEBUG_SERVER

#pragma once

#include "System/DebugServer/Render/ImageOps.h"

#include <rapidjson/document.h>
#include <string>

namespace AV{
    /**
    Serialises an already-captured frame in agent-friendly text forms.

    Pure transform over a CapturedFrame: no engine access, so it runs safely on the
    HTTP thread (the capture itself is FrameCapture's job).
    */
    class RenderInspector{
    public:
        struct FrameParams{
            //"stats", "grid", "ascii" or "png".
            std::string form = "stats";
            //Cell counts for grid/ascii.
            int w = 32;
            int h = 18;
            //Largest output dimension for png.
            int maxDim = 512;
            //Normalised crop region; applied before downsampling when hasRegion is set.
            bool hasRegion = false;
            float regionX = 0.0f, regionY = 0.0f, regionW = 1.0f, regionH = 1.0f;
        };

        /**
        True if form names a supported output form. The route handler validates before
        capturing so a bad request never costs a GPU readback.
        */
        static bool validForm(const std::string& form);

        /**
        Populate doc from the captured frame according to params.

        @param status Set to 400 for bad params.
        */
        static void writeFrame(rapidjson::Document& doc, int& status, const FrameParams& params, const CapturedFrame& captured);

        //Bounds applied to FrameParams (public for tests and the route handler).
        static const int MAX_CELLS_X = 96;
        static const int MAX_CELLS_Y = 54;
        static const int MAX_PNG_DIM = 1024;
    };
}

#endif
