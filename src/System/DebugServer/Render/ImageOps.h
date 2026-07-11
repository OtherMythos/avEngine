#ifdef DEBUG_SERVER

#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace AV{
    /**
    A CPU-side copy of a captured frame: tightly packed RGB8, top-left origin.
    Produced by FrameCapture, consumed by every ImageOps function.
    */
    struct CapturedFrame{
        uint32_t width = 0;
        uint32_t height = 0;
        std::vector<uint8_t> rgb;
        //The rendered-frame count at capture time (from FrameCapture).
        uint64_t frameNumber = 0;

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
        CapturedFrame boxDownsample(const CapturedFrame& frame, uint32_t outW, uint32_t outH);

        /**
        Rec.709 luminance of one pixel, 0-1.
        */
        float luminance(uint8_t r, uint8_t g, uint8_t b);

        struct DominantColour{
            uint8_t rgb[3];
            float pct;   //Percentage of pixels, 0-100.
        };

        struct FrameStats{
            uint8_t meanRgb[3];
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
        std::vector<uint8_t> encodePng(const CapturedFrame& frame);

        /**
        Standard base64 (RFC 4648, with padding).
        */
        std::string base64(const std::vector<uint8_t>& data);
    }
}

#endif
