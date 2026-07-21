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

        /**
        Downsample a capture to analysis resolution (at most ANALYSIS_W x ANALYSIS_H).
        Hashing, diffing and snapshot storage all work at this scale so results are
        comparable and memory stays bounded regardless of the display resolution.
        */
        static CapturedFrame toAnalysisFrame(const CapturedFrame& captured);

        /** { "frame": n, "dhash": "..." } for a live capture. */
        static void writeHash(rapidjson::Document& doc, const CapturedFrame& analysis);

        /** Snapshot confirmation: name, frame number, dhash. */
        static void writeSnapshot(rapidjson::Document& doc, const std::string& name, const CapturedFrame& analysis);

        /** Difference between two analysis frames. */
        static void writeCompare(rapidjson::Document& doc, const CapturedFrame& a, const CapturedFrame& b,
                                 int gridW, int gridH, float threshold);

        /** Connected regions matching a colour, in normalised coordinates. */
        static void writeFind(rapidjson::Document& doc, const CapturedFrame& analysis,
                              uint8_t r, uint8_t g, uint8_t b, int tolerance);

        //Bounds applied to FrameParams (public for tests and the route handler).
        static const int MAX_CELLS_X = 96;
        static const int MAX_CELLS_Y = 54;
        static const int MAX_PNG_DIM = 1024;
        //Analysis resolution cap.
        static const int ANALYSIS_W = 320;
        static const int ANALYSIS_H = 180;
        //Most changed cells reported by writeCompare.
        static const int MAX_CHANGED_CELLS = 24;
    };
}

#endif
