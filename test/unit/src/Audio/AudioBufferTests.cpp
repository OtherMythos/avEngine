#include <gtest/gtest.h>
#include "Audio/AudioBuffer.h"
#include "Audio/AudioSource.h"
#include "Audio/OpenAL/AudioManagerOpenAL.h"
#include "AudioTestFiles.h"
#include "unit/src/TestAccessors.h"

namespace AV{
    namespace{
        class AudioBufferTest : public testing::Test{
        protected:
            AudioManagerOpenAL manager;
            void SetUp() override{
                const bool previous = TestableSystemSettings::mSimulationAudio;
                TestableSystemSettings::mSimulationAudio = true;
                manager.setup();
                TestableSystemSettings::mSimulationAudio = previous;
                ASSERT_TRUE(manager.isSetup());
            }
            void TearDown() override{ manager.shutdown(); }
        };
    }

    TEST_F(AudioBufferTest, UnloadedMetadataIsUnknown){
        auto buffer = manager.createAudioBuffer();
        EXPECT_FALSE(buffer->isReady());
        EXPECT_DOUBLE_EQ(buffer->getDurationSeconds(), -1);
        EXPECT_TRUE(buffer->getPath().empty());
        EXPECT_TRUE(buffer->getError().empty());
    }

    TEST_F(AudioBufferTest, LoadedMetadataIsAvailableThroughBufferAndSource){
        AudioTestFile file(6000);
        auto source = manager.createAudioSource(file.path);
        auto buffer = source->getAudioBuffer();
        ASSERT_TRUE(buffer->isReady());
        EXPECT_DOUBLE_EQ(buffer->getDurationSeconds(), 0.125);
        EXPECT_EQ(buffer->getPath(), file.path);
        EXPECT_TRUE(buffer->getError().empty());
        EXPECT_DOUBLE_EQ(source->getDurationSeconds(), 0.125);
        EXPECT_EQ(source->getAudioPath(), file.path);
        EXPECT_TRUE(source->getError().empty());
    }

    TEST_F(AudioBufferTest, FailedLoadReportsErrorAndSuccessfulRetryClearsIt){
        AudioTestFile file;
        auto source = manager.createAudioSource(file.path + ".missing");
        auto buffer = source->getAudioBuffer();
        EXPECT_FALSE(buffer->isReady());
        EXPECT_DOUBLE_EQ(buffer->getDurationSeconds(), -1);
        EXPECT_EQ(buffer->getPath(), file.path + ".missing");
        EXPECT_FALSE(buffer->getError().empty());
        EXPECT_EQ(source->getPlaybackState(), "failed");
        EXPECT_EQ(source->getError(), buffer->getError());
        source = nullptr;
        buffer->load(file.path);
        EXPECT_TRUE(buffer->isReady());
        EXPECT_TRUE(buffer->getError().empty());
        EXPECT_EQ(buffer->getPath(), file.path);
        EXPECT_DOUBLE_EQ(buffer->getDurationSeconds(), 0.1);
    }

    TEST_F(AudioBufferTest, FailedReloadPreservesLoadedMetadata){
        AudioTestFile file;
        auto buffer = manager.createAudioBuffer();
        buffer->load(file.path);
        ASSERT_TRUE(buffer->isReady());
        const auto handle = buffer->getBuffer();
        buffer->load(file.path + ".missing");
        EXPECT_TRUE(buffer->isReady());
        EXPECT_EQ(buffer->getBuffer(), handle);
        EXPECT_EQ(buffer->getPath(), file.path);
        EXPECT_DOUBLE_EQ(buffer->getDurationSeconds(), 0.1);
        EXPECT_FALSE(buffer->getError().empty());
    }

    TEST_F(AudioBufferTest, StaticSourceReportsMetadataAndNaturalEnd){
        AudioTestFile file(200);
        auto source = manager.createAudioSource(file.path);
        EXPECT_EQ(source->getPlaybackState(), "ready");
        EXPECT_NEAR(source->getDurationSeconds(), 200.0 / 48000, 0.00001);
        source->play();
        manager.simulationTick(240);
        EXPECT_EQ(source->getPlaybackState(), "ended");
        EXPECT_NEAR(source->getPositionSeconds(), source->getDurationSeconds(), 0.00001);
        source->stop();
        EXPECT_EQ(source->getPlaybackState(), "stopped");
    }

    TEST_F(AudioBufferTest, PositionTracksPauseSeekAndResume){
        AudioTestFile file(48000);
        auto source = manager.createAudioSource(file.path);
        source->play();
        manager.simulationTick(240);
        const auto pausedPosition = source->getPositionSeconds();
        EXPECT_NEAR(pausedPosition, 200.0 / 48000, 0.0001);
        source->pause();
        manager.simulationTick(240);
        EXPECT_EQ(source->getPlaybackState(), "paused");
        EXPECT_DOUBLE_EQ(source->getPositionSeconds(), pausedPosition);
        source->seekSeconds(0.5f);
        EXPECT_NEAR(source->getPositionSeconds(), 0.5, 0.0001);
        source->play();
        manager.simulationTick(240);
        EXPECT_EQ(source->getPlaybackState(), "playing");
        EXPECT_NEAR(source->getPositionSeconds(), 0.5 + 200.0 / 48000, 0.0001);
        source->stop();
        EXPECT_EQ(source->getPlaybackState(), "stopped");
        EXPECT_DOUBLE_EQ(source->getPositionSeconds(), 0);
    }
}
