#include "Video/MpegVideo.h"

#include "RedBlueMpeg.h"

#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

namespace {
    class MpegVideo : public ::testing::Test{
    protected:
        void SetUp() override{
            const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
            const auto directory = std::filesystem::temp_directory_path() / ("av-mpeg-" + std::to_string(stamp));
            ASSERT_TRUE(std::filesystem::create_directory(directory));
            mDirectory = directory;

            mVideoPath = (mDirectory / "red-blue.mpg").string();
            std::ofstream file(mVideoPath, std::ios::binary);
            file.write(AV::kRedBlueMpeg, sizeof(AV::kRedBlueMpeg) - 1);
            file.close();
            ASSERT_TRUE(file.good());
        }

        void TearDown() override{
            if(mDirectory.empty()) return;

            std::error_code error;
            std::filesystem::remove_all(mDirectory, error);
            EXPECT_FALSE(error) << error.message();
        }

        std::filesystem::path mDirectory;
        std::string mVideoPath;
    };

    TEST_F(MpegVideo, DecodesBFramesAndRewinds){
        AV::MpegVideo video(mVideoPath);
        ASSERT_EQ(video.getWidth(), 35u);
        ASSERT_EQ(video.getHeight(), 19u);
        ASSERT_DOUBLE_EQ(video.getFrameRate(), 25.0);
        EXPECT_NEAR(video.getPixelAspectRatio(), 196.0 / 201.0, 0.0001);

        const auto stride = video.getWidth() * 4 + 16;
        std::vector<std::uint8_t> pixels(stride * video.getHeight(), 0);
        const auto corner = stride * (video.getHeight() - 1) + (video.getWidth() - 1) * 4;
        video.writeRgba(pixels.data(), stride);
        EXPECT_GT(pixels[corner], 240);
        EXPECT_LT(pixels[corner + 2], 16);
        EXPECT_EQ(pixels[corner + 3], 255);
        EXPECT_EQ(pixels[corner + 4], 0);
        for(unsigned channel = 0; channel < 4; ++channel){
            EXPECT_EQ(pixels[channel], pixels[corner + channel]);
        }
        const auto firstFrame = pixels;

        EXPECT_FALSE(video.advance(0.0));
        EXPECT_FALSE(video.advance(0.02));
        ASSERT_TRUE(video.advance(0.02));
        for(int frame = 2; frame < 6; ++frame){
            ASSERT_TRUE(video.advance(0.04)) << "Frame " << frame;
        }

        video.writeRgba(pixels.data(), stride);
        EXPECT_LT(pixels[corner], 16);
        EXPECT_GT(pixels[corner + 2], 240);
        EXPECT_FALSE(video.hasEnded());
        EXPECT_FALSE(video.advance(0.04));
        EXPECT_TRUE(video.hasEnded());
        EXPECT_FALSE(video.advance(0.04));

        video.rewind();
        EXPECT_FALSE(video.hasEnded());
        video.writeRgba(pixels.data(), stride);
        EXPECT_EQ(pixels, firstFrame);
    }

    TEST_F(MpegVideo, RejectsMissingAndNonVideoFiles){
        const auto invalidPath = mDirectory / "invalid.mpg";
        std::ofstream file(invalidPath, std::ios::binary);
        file << "This file contains no MPEG video.";
        file.close();
        ASSERT_TRUE(file.good());
        EXPECT_THROW(AV::MpegVideo{(mDirectory / "missing.mpg").string()}, std::runtime_error);
        EXPECT_THROW(AV::MpegVideo{invalidPath.string()}, std::runtime_error);
    }

    TEST_F(MpegVideo, BoundsUpdatesAndRejectsInvalidDeltas){
        AV::MpegVideo video(mVideoPath);
        EXPECT_THROW(video.advance(-0.1), std::invalid_argument);
        EXPECT_THROW(video.advance(std::numeric_limits<double>::infinity()), std::invalid_argument);
        EXPECT_THROW(video.advance(std::numeric_limits<double>::quiet_NaN()), std::invalid_argument);
        EXPECT_TRUE(video.advance(60.0));
        EXPECT_TRUE(video.advance(0.001));
        EXPECT_FALSE(video.advance(0.001));
        EXPECT_FALSE(video.hasEnded());
    }
}
