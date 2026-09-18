#ifdef DEBUG_SERVER

#pragma once

#include <vector>
#include <string>
#include "System/EnginePrerequisites.h"

namespace AV{
    /**
    A CPU-side copy of a captured frame: tightly packed RGB8, top-left origin.
    Produced by FrameCapture, consumed by every ImageOps function.
    */
    struct CapturedFrame{
        uint32 width = 0;
        uint32 height = 0;
        std::vector<uint8> rgb;
        //The rendered-frame count at capture time (from FrameCapture).
        uint64 frameNumber = 0;

        bool valid() const { return width > 0 && height > 0 && rgb.size() == static_cast<size_t>(width) * height * 3; }
    };

    /**
    Pure pixel operations over CapturedFrame buffers.

    No engine, Ogre or rapidjson dependencies, so everything here is unit testable
    (and verifiable standalone) with synthetic frames.
    */
    namespace ImageOps{
        /**
        Crop by a normalised region (values 0-1, clamped). Returns an empty frame if the
        clamped region degenerates to zero pixels.
        */
        CapturedFrame crop(const CapturedFrame& frame, float x, float y, float w, float h);

        /**
        Box-filter downsample to exactly outW x outH cells. Each output pixel is the
        mean of its source rectangle. Passing dimensions >= the source returns a copy
        clamped to the source size (this function never upsamples).
        */
        CapturedFrame boxDownsample(const CapturedFrame& frame, uint32 outW, uint32 outH);

        /**
        Rec.709 luminance of one pixel, 0-1.
        */
        float luminance(uint8 r, uint8 g, uint8 b);

        struct DominantColour{
            uint8 rgb[3];
            float pct;   //Percentage of pixels, 0-100.
        };

        struct FrameStats{
            uint8 meanRgb[3];
            float lumMean;
            float lumMin;
            float lumMax;
            //Percentage of pixels per luminance decile, 0-100 each.
            float histogram[10];
            //Up to 5 entries, largest first.
            std::vector<DominantColour> dominantColours;
        };

        FrameStats computeStats(const CapturedFrame& frame);

        /**
        One string per row, one character per pixel, from a 10-step luminance ramp
        (dark " .:-=+*#%@" bright). Downsample first; this maps pixels 1:1.
        */
        std::vector<std::string> asciiArt(const CapturedFrame& frame);

        /**
        One string per row, 6 lowercase hex chars (rrggbb) per pixel.
        */
        std::vector<std::string> hexRows(const CapturedFrame& frame);

        /**
        Encode as PNG (via stb_image_write). Returns an empty vector on failure.
        */
        std::vector<uint8> encodePng(const CapturedFrame& frame);

        /**
        Standard base64 (RFC 4648, with padding).
        */
        std::string base64(const std::vector<uint8>& data);

        /**
        64-bit perceptual difference hash. The frame is reduced to a 9x8 luminance grid;
        each bit records whether a cell is brighter than its right-hand neighbour, so the
        hash tracks structure rather than absolute colour and survives small rendering
        noise. Hamming distance between two hashes is a meaningful similarity metric
        (0 identical, >10 clearly different).
        */
        uint64 dHash(const CapturedFrame& frame);

        /** Number of differing bits between two hashes, 0-64. */
        int hammingDistance(uint64 a, uint64 b);

        /** 16 lowercase hex chars. */
        std::string hashToHex(uint64 hash);

        struct DiffCell{
            uint32 x, y;
            //Mean absolute RGB difference for this cell, 0-1.
            float delta;
        };

        struct DiffResult{
            bool valid = false;
            uint32 gridW = 0, gridH = 0;
            //Fraction of cells whose delta exceeded the threshold, 0-1.
            float changedFraction = 0.0f;
            //Mean delta across every cell, 0-1.
            float meanDelta = 0.0f;
            //Changed cells, largest delta first (caller caps how many are reported).
            std::vector<DiffCell> changedCells;
            //Normalised bounding box enclosing every changed cell.
            bool hasRegion = false;
            float regionX = 0.0f, regionY = 0.0f, regionW = 0.0f, regionH = 0.0f;
        };

        /**
        Per-cell mean absolute difference between two frames on a gridW x gridH grid.
        The frames are box-downsampled to the grid first, so they need not share
        dimensions. threshold is the per-cell delta (0-1) above which a cell counts as
        changed.
        */
        DiffResult diff(const CapturedFrame& a, const CapturedFrame& b,
                        uint32 gridW, uint32 gridH, float threshold);

        struct ColourMatch{
            //Normalised centroid and bounding box of the connected region.
            float centreX, centreY;
            float bboxX, bboxY, bboxW, bboxH;
            //Percentage of the frame's pixels belonging to this region, 0-100.
            float pct;
        };

        /**
        Find connected regions whose colour is within tolerance of the target, using
        per-channel (Chebyshev) distance. Regions smaller than minPixels are discarded.
        Results are largest first, capped at maxMatches.
        */
        std::vector<ColourMatch> findColour(const CapturedFrame& frame,
                                            uint8 r, uint8 g, uint8 b,
                                            int tolerance, size_t minPixels, size_t maxMatches);
    }
}

#endif
