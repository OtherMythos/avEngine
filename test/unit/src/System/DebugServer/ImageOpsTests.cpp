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
