#ifdef DEBUG_SERVER

#include "gtest/gtest.h"

#include "System/DebugServer/Render/ImageOps.h"

#include <cstring>

using namespace AV;

namespace{
    CapturedFrame solidFrame(uint32_t w, uint32_t h, uint8_t r, uint8_t g, uint8_t b){
        CapturedFrame frame;
        frame.width = w;
        frame.height = h;
        frame.rgb.resize(static_cast<size_t>(w) * h * 3);
        for(size_t i = 0; i < static_cast<size_t>(w) * h; i++){
            frame.rgb[i * 3 + 0] = r;
            frame.rgb[i * 3 + 1] = g;
            frame.rgb[i * 3 + 2] = b;
        }
        return frame;
    }

    //Left half one colour, right half another.
    CapturedFrame splitFrame(uint32_t w, uint32_t h, uint8_t leftVal, uint8_t rightVal){
        CapturedFrame frame = solidFrame(w, h, leftVal, leftVal, leftVal);
        for(uint32_t y = 0; y < h; y++){
            for(uint32_t x = w / 2; x < w; x++){
                uint8_t* p = &frame.rgb[(static_cast<size_t>(y) * w + x) * 3];
                p[0] = p[1] = p[2] = rightVal;
            }
        }
        return frame;
    }
}

TEST(ImageOpsTests, boxDownsampleAveragesExactly){
    //4x4 checkerboard of 0 and 255 downsampled to 1x1 must average to ~127.
    CapturedFrame frame = solidFrame(4, 4, 0, 0, 0);
    for(uint32_t y = 0; y < 4; y++){
        for(uint32_t x = 0; x < 4; x++){
            if((x + y) % 2 == 0){
                uint8_t* p = &frame.rgb[(static_cast<size_t>(y) * 4 + x) * 3];
                p[0] = p[1] = p[2] = 255;
            }
        }
    }

    CapturedFrame out = ImageOps::boxDownsample(frame, 1, 1);
    ASSERT_EQ(out.width, 1u);
    ASSERT_EQ(out.height, 1u);
    ASSERT_EQ(out.rgb[0], 127);
}

TEST(ImageOpsTests, boxDownsamplePreservesSpatialLayout){
    CapturedFrame frame = splitFrame(64, 32, 0, 255);
    CapturedFrame out = ImageOps::boxDownsample(frame, 2, 1);

    ASSERT_EQ(out.width, 2u);
    ASSERT_EQ(out.height, 1u);
    ASSERT_EQ(out.rgb[0], 0);     //left cell dark
    ASSERT_EQ(out.rgb[3], 255);   //right cell bright
}

TEST(ImageOpsTests, boxDownsampleNeverUpsamples){
    CapturedFrame frame = solidFrame(8, 8, 10, 20, 30);
    CapturedFrame out = ImageOps::boxDownsample(frame, 100, 100);
    ASSERT_EQ(out.width, 8u);
    ASSERT_EQ(out.height, 8u);
    ASSERT_EQ(out.rgb, frame.rgb);
}

TEST(ImageOpsTests, cropExtractsRegion){
    CapturedFrame frame = splitFrame(100, 50, 0, 200);

    //Right half only.
    CapturedFrame right = ImageOps::crop(frame, 0.5f, 0.0f, 0.5f, 1.0f);
    ASSERT_EQ(right.width, 50u);
    ASSERT_EQ(right.height, 50u);
    for(size_t i = 0; i < right.rgb.size(); i++){
        ASSERT_EQ(right.rgb[i], 200);
    }
}

TEST(ImageOpsTests, cropDegenerateRegionReturnsInvalid){
    CapturedFrame frame = solidFrame(10, 10, 1, 2, 3);
    CapturedFrame out = ImageOps::crop(frame, 1.0f, 0.0f, 0.5f, 1.0f);
    ASSERT_FALSE(out.valid());
}

TEST(ImageOpsTests, statsOnSolidFrame){
    CapturedFrame frame = solidFrame(32, 32, 255, 0, 0);
    ImageOps::FrameStats stats = ImageOps::computeStats(frame);

    ASSERT_EQ(stats.meanRgb[0], 255);
    ASSERT_EQ(stats.meanRgb[1], 0);
    ASSERT_EQ(stats.meanRgb[2], 0);
    ASSERT_FLOAT_EQ(stats.lumMin, stats.lumMax);

    //One dominant colour covering 100% of the frame.
    ASSERT_EQ(stats.dominantColours.size(), 1u);
    ASSERT_EQ(stats.dominantColours[0].rgb[0], 255);
    ASSERT_FLOAT_EQ(stats.dominantColours[0].pct, 100.0f);

    //Histogram sums to ~100%.
    float total = 0.0f;
    for(int i = 0; i < 10; i++) total += stats.histogram[i];
    ASSERT_NEAR(total, 100.0f, 0.01f);
}

TEST(ImageOpsTests, statsDominantColoursOrderedByCoverage){
    //75% grey, 25% white.
    CapturedFrame frame = solidFrame(4, 4, 100, 100, 100);
    for(uint32_t i = 0; i < 4; i++){
        uint8_t* p = &frame.rgb[i * 3];
        p[0] = p[1] = p[2] = 250;
    }
    ImageOps::FrameStats stats = ImageOps::computeStats(frame);

    ASSERT_EQ(stats.dominantColours.size(), 2u);
    ASSERT_EQ(stats.dominantColours[0].rgb[0], 100);
    ASSERT_FLOAT_EQ(stats.dominantColours[0].pct, 75.0f);
    ASSERT_EQ(stats.dominantColours[1].rgb[0], 250);
    ASSERT_FLOAT_EQ(stats.dominantColours[1].pct, 25.0f);
}

TEST(ImageOpsTests, asciiRampMonotonicInLuminance){
    //A 10x1 frame stepping from black to white must produce non-decreasing ramp chars.
    static const char ramp[] = " .:-=+*#%@";
    CapturedFrame frame = solidFrame(10, 1, 0, 0, 0);
    for(uint32_t x = 0; x < 10; x++){
        uint8_t v = static_cast<uint8_t>(x * 255 / 9);
        uint8_t* p = &frame.rgb[x * 3];
        p[0] = p[1] = p[2] = v;
    }

    std::vector<std::string> rows = ImageOps::asciiArt(frame);
    ASSERT_EQ(rows.size(), 1u);
    ASSERT_EQ(rows[0].size(), 10u);
    ASSERT_EQ(rows[0][0], ' ');
    ASSERT_EQ(rows[0][9], '@');
    int prevIdx = -1;
    for(char c : rows[0]){
        const char* pos = strchr(ramp, c);
        ASSERT_NE(pos, nullptr);
        int idx = static_cast<int>(pos - ramp);
        ASSERT_GE(idx, prevIdx);
        prevIdx = idx;
    }
}

TEST(ImageOpsTests, hexRowsRoundTripKnownValues){
    CapturedFrame frame = solidFrame(2, 1, 0xAB, 0xCD, 0xEF);
    std::vector<std::string> rows = ImageOps::hexRows(frame);

    ASSERT_EQ(rows.size(), 1u);
    ASSERT_EQ(rows[0], "abcdefabcdef");
}

TEST(ImageOpsTests, pngEncodeProducesValidSignature){
    CapturedFrame frame = solidFrame(16, 16, 40, 90, 200);
    std::vector<uint8_t> png = ImageOps::encodePng(frame);

    ASSERT_GT(png.size(), 8u);
    //PNG magic bytes.
    const uint8_t signature[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
    for(int i = 0; i < 8; i++){
        ASSERT_EQ(png[i], signature[i]);
    }
}

TEST(ImageOpsTests, dHashIsStableUnderSmallNoise){
    //A gradient gives the hash real structure to encode.
    CapturedFrame frame = solidFrame(64, 64, 0, 0, 0);
    for(uint32_t y = 0; y < 64; y++){
        for(uint32_t x = 0; x < 64; x++){
            uint8_t* p = &frame.rgb[(static_cast<size_t>(y) * 64 + x) * 3];
            p[0] = p[1] = p[2] = static_cast<uint8_t>(x * 4);
        }
    }

    //The same image with +-2 per-channel noise must hash near-identically.
    CapturedFrame noisy = frame;
    for(size_t i = 0; i < noisy.rgb.size(); i++){
        const int delta = (i % 2 == 0) ? 2 : -2;
        const int v = static_cast<int>(noisy.rgb[i]) + delta;
        noisy.rgb[i] = static_cast<uint8_t>(v < 0 ? 0 : (v > 255 ? 255 : v));
    }

    const uint64_t a = ImageOps::dHash(frame);
    const uint64_t b = ImageOps::dHash(noisy);
    ASSERT_LE(ImageOps::hammingDistance(a, b), 2);
}

TEST(ImageOpsTests, dHashDistinguishesDifferentImages){
    //Each bit records "is this cell brighter than the one to its right", so a gradient
    //that brightens left-to-right and one that darkens are maximally different: the
    //former sets no bits, the latter sets all of them.
    CapturedFrame ascending = solidFrame(64, 64, 0, 0, 0);
    CapturedFrame descending = solidFrame(64, 64, 0, 0, 0);
    for(uint32_t y = 0; y < 64; y++){
        for(uint32_t x = 0; x < 64; x++){
            const uint8_t v = static_cast<uint8_t>(x * 4);
            uint8_t* a = &ascending.rgb[(static_cast<size_t>(y) * 64 + x) * 3];
            a[0] = a[1] = a[2] = v;
            uint8_t* d = &descending.rgb[(static_cast<size_t>(y) * 64 + (63 - x)) * 3];
            d[0] = d[1] = d[2] = v;
        }
    }

    const int distance = ImageOps::hammingDistance(ImageOps::dHash(ascending), ImageOps::dHash(descending));
    ASSERT_GT(distance, 32);
}

TEST(ImageOpsTests, hashToHexIs16Chars){
    ASSERT_EQ(ImageOps::hashToHex(0).size(), 16u);
    ASSERT_EQ(ImageOps::hashToHex(0), "0000000000000000");
    ASSERT_EQ(ImageOps::hashToHex(0xa5f0c3e19b02d4f7ULL), "a5f0c3e19b02d4f7");
}

TEST(ImageOpsTests, diffOfIdenticalFramesIsZero){
    CapturedFrame frame = splitFrame(64, 32, 20, 200);
    ImageOps::DiffResult result = ImageOps::diff(frame, frame, 16, 8, 0.05f);

    ASSERT_TRUE(result.valid);
    ASSERT_FLOAT_EQ(result.changedFraction, 0.0f);
    ASSERT_FLOAT_EQ(result.meanDelta, 0.0f);
    ASSERT_TRUE(result.changedCells.empty());
    ASSERT_FALSE(result.hasRegion);
}

TEST(ImageOpsTests, diffLocalisesAChangedCorner){
    CapturedFrame before = solidFrame(64, 64, 0, 0, 0);
    CapturedFrame after = before;
    //Light up the bottom-right quadrant only.
    for(uint32_t y = 32; y < 64; y++){
        for(uint32_t x = 32; x < 64; x++){
            uint8_t* p = &after.rgb[(static_cast<size_t>(y) * 64 + x) * 3];
            p[0] = p[1] = p[2] = 255;
        }
    }

    ImageOps::DiffResult result = ImageOps::diff(before, after, 8, 8, 0.05f);
    ASSERT_TRUE(result.valid);
    //A quarter of the frame changed.
    ASSERT_NEAR(result.changedFraction, 0.25f, 0.01f);
    ASSERT_TRUE(result.hasRegion);
    //Bounding box should be the bottom-right quadrant.
    ASSERT_NEAR(result.regionX, 0.5f, 0.01f);
    ASSERT_NEAR(result.regionY, 0.5f, 0.01f);
    ASSERT_NEAR(result.regionW, 0.5f, 0.01f);
    ASSERT_NEAR(result.regionH, 0.5f, 0.01f);
}

TEST(ImageOpsTests, diffHandlesDifferentSizedFrames){
    //Same content, different resolutions: reducing both to one grid must show no change.
    CapturedFrame small = solidFrame(32, 32, 70, 80, 90);
    CapturedFrame large = solidFrame(128, 128, 70, 80, 90);

    ImageOps::DiffResult result = ImageOps::diff(small, large, 8, 8, 0.05f);
    ASSERT_TRUE(result.valid);
    ASSERT_FLOAT_EQ(result.changedFraction, 0.0f);
}

TEST(ImageOpsTests, findColourLocatesASquare){
    CapturedFrame frame = solidFrame(100, 100, 0, 0, 0);
    //Red square spanning x[20,40) y[60,80) => centre (0.30, 0.70).
    for(uint32_t y = 60; y < 80; y++){
        for(uint32_t x = 20; x < 40; x++){
            uint8_t* p = &frame.rgb[(static_cast<size_t>(y) * 100 + x) * 3];
            p[0] = 255; p[1] = 0; p[2] = 0;
        }
    }

    std::vector<ImageOps::ColourMatch> matches = ImageOps::findColour(frame, 255, 0, 0, 40, 4, 10);
    ASSERT_EQ(matches.size(), 1u);
    ASSERT_NEAR(matches[0].centreX, 0.295f, 0.02f);
    ASSERT_NEAR(matches[0].centreY, 0.695f, 0.02f);
    ASSERT_NEAR(matches[0].bboxX, 0.20f, 0.02f);
    ASSERT_NEAR(matches[0].bboxY, 0.60f, 0.02f);
    ASSERT_NEAR(matches[0].bboxW, 0.20f, 0.02f);
    //400 of 10000 pixels.
    ASSERT_NEAR(matches[0].pct, 4.0f, 0.1f);
}

TEST(ImageOpsTests, findColourSeparatesDisconnectedRegionsAndSortsBySize){
    CapturedFrame frame = solidFrame(100, 100, 0, 0, 0);
    //Small green blob top-left (10x10), larger one bottom-right (20x20), not touching.
    for(uint32_t y = 0; y < 10; y++)
        for(uint32_t x = 0; x < 10; x++){
            uint8_t* p = &frame.rgb[(static_cast<size_t>(y) * 100 + x) * 3];
            p[0] = 0; p[1] = 255; p[2] = 0;
        }
    for(uint32_t y = 70; y < 90; y++)
        for(uint32_t x = 70; x < 90; x++){
            uint8_t* p = &frame.rgb[(static_cast<size_t>(y) * 100 + x) * 3];
            p[0] = 0; p[1] = 255; p[2] = 0;
        }

    std::vector<ImageOps::ColourMatch> matches = ImageOps::findColour(frame, 0, 255, 0, 40, 4, 10);
    ASSERT_EQ(matches.size(), 2u);
    //Largest first.
    ASSERT_GT(matches[0].pct, matches[1].pct);
    ASSERT_NEAR(matches[0].centreX, 0.795f, 0.02f);
    ASSERT_NEAR(matches[1].centreX, 0.045f, 0.02f);
}

TEST(ImageOpsTests, findColourIgnoresRegionsBelowMinPixels){
    CapturedFrame frame = solidFrame(50, 50, 0, 0, 0);
    //A single stray blue pixel.
    uint8_t* p = &frame.rgb[(static_cast<size_t>(25) * 50 + 25) * 3];
    p[0] = 0; p[1] = 0; p[2] = 255;

    ASSERT_TRUE(ImageOps::findColour(frame, 0, 0, 255, 20, 4, 10).empty());
    ASSERT_EQ(ImageOps::findColour(frame, 0, 0, 255, 20, 1, 10).size(), 1u);
}

TEST(ImageOpsTests, base64KnownVectors){
    //RFC 4648 test vectors.
    ASSERT_EQ(ImageOps::base64({}), "");
    ASSERT_EQ(ImageOps::base64({'f'}), "Zg==");
    ASSERT_EQ(ImageOps::base64({'f', 'o'}), "Zm8=");
    ASSERT_EQ(ImageOps::base64({'f', 'o', 'o'}), "Zm9v");
    ASSERT_EQ(ImageOps::base64({'f', 'o', 'o', 'b'}), "Zm9vYg==");
    ASSERT_EQ(ImageOps::base64({'f', 'o', 'o', 'b', 'a'}), "Zm9vYmE=");
    ASSERT_EQ(ImageOps::base64({'f', 'o', 'o', 'b', 'a', 'r'}), "Zm9vYmFy");
}

#endif
