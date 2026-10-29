#include <gtest/gtest.h>
#include "Audio/AudioBuffer.h"
#include "Audio/OpenAL/AudioManagerOpenAL.h"
#include "Audio/OpenAL/AudioSourceOpenAL.h"
#include "AudioTestFiles.h"
#include "unit/src/TestAccessors.h"

namespace AV{
    namespace{
        void openLoopback(AudioManagerOpenAL& manager){
            const bool previous = TestableSystemSettings::mSimulationAudio;
            TestableSystemSettings::mSimulationAudio = true;
            manager.setup();
            TestableSystemSettings::mSimulationAudio = previous;
        }

        class NativeSource : public AudioSourceOpenAL{
        public:
            using AudioSourceOpenAL::AudioSourceOpenAL;
            ALuint handle() const { return mSource; }
        };

        class AudioLifetimeTest : public testing::Test{
        protected:
            AudioManagerOpenAL manager;
            void SetUp() override{
                openLoopback(manager);
                ASSERT_TRUE(manager.isSetup());
            }
            void TearDown() override{ manager.shutdown(); }
        };
    }

    TEST_F(AudioLifetimeTest, DeletesNativeSourcesAndBuffers){
        AudioTestFile file;
        auto buffer = manager.createAudioBuffer();
        buffer->load(file.path);
        auto source = std::make_shared<NativeSource>(&manager);
        source->setAudioBuffer(buffer);
        const auto sourceHandle = source->handle();
        const auto bufferHandle = buffer->getBuffer();
        buffer.reset();
        EXPECT_TRUE(alIsSource(sourceHandle));
        EXPECT_TRUE(alIsBuffer(bufferHandle));
        source.reset();
        EXPECT_FALSE(alIsSource(sourceHandle));
        EXPECT_FALSE(alIsBuffer(bufferHandle));
        EXPECT_EQ(manager.getNumAudioSources(), 0);
        EXPECT_EQ(manager.getNumAudioBuffers(), 0);
    }

    TEST_F(AudioLifetimeTest, FailedReloadPreservesOldBufferAndSuccessfulReloadDeletesIt){
        AudioTestFile file;
        auto buffer = manager.createAudioBuffer();
        buffer->load(file.path);
        const auto old = buffer->getBuffer();
        buffer->load("/does-not-exist.wav");
        EXPECT_EQ(buffer->getBuffer(), old);
        EXPECT_TRUE(buffer->isReady());
        EXPECT_FALSE(buffer->getError().empty());
        buffer->load(file.path);
        EXPECT_NE(buffer->getBuffer(), old);
        EXPECT_FALSE(alIsBuffer(old));
        EXPECT_TRUE(buffer->getError().empty());
    }

    TEST_F(AudioLifetimeTest, ReloadWhileReferencedIsRejected){
        AudioTestFile file;
        auto source = manager.createAudioSource(file.path);
        auto buffer = source->getAudioBuffer();
        const auto handle = buffer->getBuffer();
        buffer->load(file.path);
        EXPECT_EQ(buffer->getBuffer(), handle);
        EXPECT_TRUE(alIsBuffer(handle));
        EXPECT_FALSE(buffer->getError().empty());
    }

    TEST_F(AudioLifetimeTest, ShutdownReleasesNativeObjectsWithSurvivingWrappers){
        AudioTestFile file;
        auto buffer = manager.createAudioBuffer();
        buffer->load(file.path);
        auto source = std::make_shared<NativeSource>(&manager);
        source->setAudioBuffer(buffer);
        source->play();
        const auto sourceHandle = source->handle();
        const auto bufferHandle = buffer->getBuffer();
        ASSERT_TRUE(alIsSource(sourceHandle));
        ASSERT_TRUE(alIsBuffer(bufferHandle));
        //Inspect deletion before the context itself is destroyed.
        manager.AudioManager::shutdown();
        EXPECT_FALSE(alIsSource(sourceHandle));
        EXPECT_FALSE(alIsBuffer(bufferHandle));
        EXPECT_FALSE(buffer->isReady());
        EXPECT_EQ(source->getPlaybackState(), "unavailable");
        EXPECT_EQ(manager.getNumAudioSources(), 1);
        EXPECT_EQ(manager.getNumAudioBuffers(), 1);
        manager.AudioManager::shutdown();
        EXPECT_EQ(alGetError(), AL_NO_ERROR);
    }

    TEST(AudioLifetime, BufferedHandlesCanOutliveTheirManager){
        AudioTestFile file;
        AudioSourcePtr source;
        AudioBufferPtr buffer;
        {
            auto native = std::make_unique<AudioManagerOpenAL>();
            openLoopback(*native);
            ASSERT_TRUE(native->isSetup());
            source = native->createAudioSource(file.path);
            buffer = source->getAudioBuffer();
            ASSERT_TRUE(buffer->isReady());
            std::unique_ptr<AudioManager> owner = std::move(native);
        }
        source->play();
        source->pause();
        source->stop();
        source->seekSeconds(0.5f);
        source->setVolume(0.5f);
        source->setAudioBuffer(nullptr);
        buffer->load(file.path);
        EXPECT_EQ(source->getPlaybackState(), "unavailable");
        EXPECT_FALSE(buffer->isReady());
        EXPECT_FALSE(buffer->getError().empty());
        source.reset();
        buffer.reset();
    }
}
