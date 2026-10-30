#include <gtest/gtest.h>
#include "Audio/OpenAL/AudioManagerOpenAL.h"
#include "Audio/OpenAL/AudioStreamSourceOpenAL.h"
#include "Audio/OpenAL/AudioBufferOpenAL.h"
#include "Threading/JobDispatcher.h"
#include "AudioTestFiles.h"
#include <algorithm>
#include <cmath>
#include <thread>
#ifdef DEBUG_SERVER
#include "System/DebugServer/Inspection/AudioInspector.h"
#endif

using namespace AV;

namespace{
    class LoopbackManager : public AudioManagerOpenAL{
    public:
        void open(){ setupDevice(true); }
    };
    class NativeStream : public AudioStreamSourceOpenAL{
    public:
        using AudioStreamSourceOpenAL::AudioStreamSourceOpenAL;
        ALuint handle() const { return mSource; }
    };
    class StreamOpenALTest : public testing::Test{
    protected:
        LoopbackManager manager;
        std::unique_ptr<AudioTestFile> mFile;
        template<typename... Args>
        AudioTestFile& makeFile(Args... args){
            mFile = std::make_unique<AudioTestFile>(args...);
            return *mFile;
        }
        void SetUp() override{
            manager.open();
            ASSERT_TRUE(manager.isSetup());
        }
        void TearDown() override{
            manager.shutdown();
            JobDispatcher::shutdown();
            mFile.reset();
            EXPECT_EQ(alGetError(), AL_INVALID_OPERATION);
        }
        std::shared_ptr<AudioStreamSourceOpenAL> create(const std::string& path){
            return std::static_pointer_cast<AudioStreamSourceOpenAL>(manager.createAudioSource(path, AudioSourceType::Stream));
        }
        void prime(const std::shared_ptr<AudioStreamSourceOpenAL>& stream){
            stream->play();
            stream->prepareSimulation();
            ASSERT_EQ(stream->getPlaybackState(), "playing") << stream->getError();
        }
        bool fill(const std::shared_ptr<AudioStreamSourceOpenAL>& stream){
            const auto end = std::chrono::steady_clock::now() + std::chrono::seconds(1);
            while(std::chrono::steady_clock::now() < end){
                manager.updateStreams();
                if(stream->getBufferedSeconds() > 0 || !stream->getError().empty()) return true;
                std::this_thread::yield();
            }
            return false;
        }
    };
}

TEST_F(StreamOpenALTest, PlaysActualSamplesWithBoundedStorage){
    auto& file = makeFile(48000);
    auto stream = create(file.path);
    prime(stream);
    manager.simulationTick(240);
    const auto& samples = *manager.simulationSamples();
    EXPECT_EQ(samples.size(), 400u);
    EXPECT_TRUE(std::any_of(samples.begin(), samples.end(), [](float x){ return std::abs(x) > 0.001f; }));
    EXPECT_NEAR(stream->getPositionSeconds(), 200.0 / 48000, 0.0001);
    EXPECT_DOUBLE_EQ(stream->getDurationSeconds(), 1);
    EXPECT_EQ(stream->getAudioBuffer(), nullptr);
    EXPECT_LT(stream->getStreamMemoryBytes(), 263000u);
    EXPECT_EQ(stream->getUnderruns(), 0u);
}

#ifdef DEBUG_SERVER
TEST_F(StreamOpenALTest, InspectorReportsStreamWithoutStaticBuffer){
    auto& file = makeFile(48000);
    manager.debugState().enabled = true;
    auto stream = create(file.path);
    prime(stream);
    manager.simulationTick(1);

    rapidjson::Document doc;
    int status = 200;
    AudioInspector::write(doc, status, manager, "source", stream->getDebugId(), AudioQuery(), false);
    ASSERT_EQ(status, 200);
    EXPECT_STREQ(doc["type"].GetString(), "stream");
    EXPECT_STREQ(doc["path"].GetString(), file.path.c_str());
    EXPECT_TRUE(doc["bufferId"].IsNull());
    EXPECT_TRUE(doc["buffer"].IsNull());
    EXPECT_TRUE(doc["error"].IsNull());
    EXPECT_DOUBLE_EQ(doc["durationSeconds"].GetDouble(), 1);
    EXPECT_DOUBLE_EQ(doc["queuedSeconds"].GetDouble(), stream->getBufferedSeconds());
    EXPECT_DOUBLE_EQ(doc["decodedSeconds"].GetDouble(), stream->getDecodedSeconds());
    EXPECT_EQ(doc["underruns"].GetUint(), 0u);
    EXPECT_EQ(doc["pcmCapacityBytes"].GetUint64(), stream->getStreamMemoryBytes());
    EXPECT_LT(doc["pcmCapacityBytes"].GetUint64(), 263000u);
    for(const auto& diagnostic : doc["diagnostics"].GetArray()){
        EXPECT_STRNE(diagnostic["code"].GetString(), "missingBuffer");
    }
    AudioQuery query;
    query.path = file.path;
    AudioInspector::write(doc, status, manager, "sources", 0, query, false);
    ASSERT_EQ(doc["total"].GetUint64(), 1u);
    EXPECT_EQ(doc["sources"][0]["id"].GetUint64(), stream->getDebugId());
}
#endif

TEST_F(StreamOpenALTest, OpeningWithoutPlayDoesNotAdvance){
    auto& file = makeFile();
    auto stream = create(file.path);
    ASSERT_TRUE(fill(stream));
    EXPECT_EQ(stream->getPlaybackState(), "ready");
    manager.simulationTick(240);
    EXPECT_DOUBLE_EQ(stream->getPositionSeconds(), 0);
}

TEST_F(StreamOpenALTest, PauseDuringOpeningDoesNotAutoStart){
    auto& file = makeFile();
    auto stream = create(file.path);
    stream->play();
    stream->pause();
    ASSERT_TRUE(fill(stream));
    manager.simulationTick(240);
    EXPECT_EQ(stream->getPlaybackState(), "paused");
    EXPECT_DOUBLE_EQ(stream->getPositionSeconds(), 0);
    prime(stream);
}

TEST_F(StreamOpenALTest, PauseAndResumePreserveCursor){
    auto& file = makeFile();
    auto stream = create(file.path);
    prime(stream);
    manager.simulationTick(240);
    const auto position = stream->getPositionSeconds();
    stream->pause();
    manager.simulationTick(240);
    EXPECT_NEAR(stream->getPositionSeconds(), position, 0.00001);
    stream->play();
    manager.simulationTick(240);
    EXPECT_GT(stream->getPositionSeconds(), position);
}

TEST_F(StreamOpenALTest, StopAndReplayStartAtBeginning){
    auto& file = makeFile();
    auto stream = create(file.path);
    prime(stream);
    manager.simulationTick(240);
    stream->stop();
    manager.updateStreams();
    EXPECT_EQ(stream->getPlaybackState(), "stopped");
    EXPECT_DOUBLE_EQ(stream->getPositionSeconds(), 0);
    EXPECT_DOUBLE_EQ(stream->getBufferedSeconds(), 0);
    prime(stream);
    EXPECT_DOUBLE_EQ(stream->getPositionSeconds(), 0);
}

TEST_F(StreamOpenALTest, StopWhileOpeningCannotBeRevivedByCompletion){
    auto& file = makeFile();
    auto stream = create(file.path);
    stream->play();
    stream->stop();
    JobDispatcher::shutdown();
    manager.updateStreams();
    EXPECT_EQ(stream->getPlaybackState(), "stopped");
    EXPECT_DOUBLE_EQ(stream->getBufferedSeconds(), 0);
}

TEST_F(StreamOpenALTest, NaturalEndWaitsForFinalQueuedSample){
    auto& file = makeFile(400);
    auto stream = create(file.path);
    prime(stream);
    EXPECT_EQ(stream->getPlaybackState(), "playing");
    manager.simulationTick(240);
    EXPECT_EQ(stream->getPlaybackState(), "playing");
    manager.simulationTick(240);
    EXPECT_EQ(stream->getPlaybackState(), "ended");
    EXPECT_NEAR(stream->getPositionSeconds(), 400.0 / 48000, 0.00001);
    EXPECT_EQ(stream->getUnderruns(), 0u);
    prime(stream);
    EXPECT_DOUBLE_EQ(stream->getPositionSeconds(), 0);
}

TEST_F(StreamOpenALTest, SeekWhilePlayingReprimesAtRequestedFrame){
    auto& file = makeFile(48000);
    auto stream = create(file.path);
    prime(stream);
    stream->seekSeconds(0.5f);
    stream->prepareSimulation();
    EXPECT_NEAR(stream->getPositionSeconds(), 0.5, 0.00001);
    manager.simulationTick(240);
    EXPECT_GT(stream->getPositionSeconds(), 0.5);
}

TEST_F(StreamOpenALTest, SimulationMixesOnlyAgainstAFullQueue){
    //Mixing against a partial queue makes the output depend on decoder timing.
    auto& file = makeFile(48000 * 2);
    auto stream = create(file.path);
    const double full = double(AudioStream::BlockCount * AudioStream::BlockFrames) / 48000;
    prime(stream);
    EXPECT_DOUBLE_EQ(stream->getBufferedSeconds(), full);
    for(int i = 0; i < 20; ++i) manager.simulationTick(60);
    stream->prepareSimulation();
    EXPECT_GT(stream->getBufferedSeconds(), full - double(AudioStream::BlockFrames) / 48000);
    stream->seekSeconds(0.5f);
    stream->prepareSimulation();
    EXPECT_DOUBLE_EQ(stream->getBufferedSeconds(), full);
}

TEST_F(StreamOpenALTest, SeekWhilePausedPreservesPause){
    auto& file = makeFile(48000);
    auto stream = create(file.path);
    prime(stream);
    stream->pause();
    stream->seekSeconds(0.25f);
    ASSERT_TRUE(fill(stream));
    manager.simulationTick(240);
    EXPECT_EQ(stream->getPlaybackState(), "paused");
    EXPECT_NEAR(stream->getPositionSeconds(), 0.25, 0.00001);
}

TEST_F(StreamOpenALTest, SeekPastEndEndsWithoutLoopingForever){
    auto& file = makeFile();
    auto stream = create(file.path);
    prime(stream);
    stream->seekSeconds(100);
    stream->prepareSimulation();
    EXPECT_EQ(stream->getPlaybackState(), "ended");
    EXPECT_NEAR(stream->getPositionSeconds(), 0.1, 0.00001);
}

TEST_F(StreamOpenALTest, LoopsAcrossPartialBlocks){
    auto& file = makeFile(127);
    auto stream = create(file.path);
    stream->setLooping(true);
    prime(stream);
    manager.simulationTick(240);
    EXPECT_EQ(stream->getPlaybackState(), "playing");
    EXPECT_NEAR(stream->getPositionSeconds(), 73.0 / 48000, 0.00001);
    EXPECT_EQ(stream->getUnderruns(), 0u);
}

TEST_F(StreamOpenALTest, MutedPlaybackStillAdvances){
    auto& file = makeFile();
    auto stream = create(file.path);
    stream->setVolume(0);
    prime(stream);
    manager.simulationTick(240);
    EXPECT_GT(stream->getPositionSeconds(), 0);
    const auto& samples = *manager.simulationSamples();
    EXPECT_TRUE(std::all_of(samples.begin(), samples.end(), [](float x){ return x == 0; }));
}

TEST_F(StreamOpenALTest, PitchChangesConsumptionRate){
    auto& file = makeFile();
    auto stream = create(file.path);
    stream->setPitch(2);
    prime(stream);
    manager.simulationTick(240);
    EXPECT_NEAR(stream->getPositionSeconds(), 400.0 / 48000, 0.0001);
}

TEST_F(StreamOpenALTest, TwoStreamsHaveIndependentCursors){
    auto& file = makeFile(48000);
    auto first = create(file.path);
    auto second = create(file.path);
    prime(first);
    prime(second);
    second->seekSeconds(0.5f);
    manager.simulationTick(240);
    EXPECT_NEAR(second->getPositionSeconds() - first->getPositionSeconds(), 0.5, 0.0001);
}

TEST_F(StreamOpenALTest, MissingFileReportsFailure){
    auto stream = create("/missing/audio.wav");
    stream->play();
    ASSERT_TRUE(fill(stream));
    manager.updateStreams();
    EXPECT_EQ(stream->getPlaybackState(), "failed");
    EXPECT_FALSE(stream->getError().empty());
}

TEST_F(StreamOpenALTest, StaticBufferCannotReplaceStreamQueue){
    auto& file = makeFile();
    auto stream = create(file.path);
    auto buffer = manager.createAudioBuffer();
    buffer->load(file.path);
    stream->setAudioBuffer(buffer);
    EXPECT_EQ(stream->getAudioBuffer(), nullptr);
    prime(stream);
}

TEST_F(StreamOpenALTest, DeletesStreamQueueBuffers){
    auto& file = makeFile();
    auto stream = std::make_shared<NativeStream>(&manager, file.path);
    prime(stream);
    const auto sourceHandle = stream->handle();
    ALint queued;
    alGetSourcei(sourceHandle, AL_BUFFERS_QUEUED, &queued);
    ASSERT_GT(queued, 0);
    std::vector<ALuint> handles(queued);
    alSourceStop(sourceHandle);
    alSourceUnqueueBuffers(sourceHandle, queued, handles.data());
    alSourceQueueBuffers(sourceHandle, queued, handles.data());
    stream.reset();
    for(auto handle : handles) EXPECT_FALSE(alIsBuffer(handle));
    EXPECT_FALSE(alIsSource(sourceHandle));
    EXPECT_EQ(alGetError(), AL_NO_ERROR);
}

TEST_F(StreamOpenALTest, ShutdownInvalidatesOutstandingHandles){
    auto& file = makeFile();
    auto stream = create(file.path);
    stream->play();
    manager.shutdown();
    stream->play();
    stream->pause();
    stream->stop();
    stream->seekSeconds(0.5);
    EXPECT_EQ(stream->getPlaybackState(), "unavailable");
    manager.open();
    EXPECT_EQ(stream->getPlaybackState(), "unavailable");
}

TEST(AudioLifetime, StreamHandlesCanOutliveTheirManager){
    AudioTestFile file;
    AudioSourcePtr stream;
    {
        LoopbackManager manager;
        manager.open();
        ASSERT_TRUE(manager.isSetup());
        stream = manager.createAudioSource(file.path, AudioSourceType::Stream);
    }
    stream->play();
    EXPECT_EQ(stream->getPlaybackState(), "unavailable");
    stream.reset();
    JobDispatcher::shutdown();
}

#include "AudioPoolGate.h"

TEST_F(StreamOpenALTest, RecoversAfterPoolStarvationWithoutFalseEnd){
    auto& file = makeFile(96000);
    auto stream = create(file.path);
    prime(stream);
    AudioPoolGate gate;
    ASSERT_TRUE(gate.wait());
    auto render = reinterpret_cast<LPALCRENDERSAMPLESSOFT>(alcGetProcAddress(nullptr, "alcRenderSamplesSOFT"));
    auto device = alcGetContextsDevice(alcGetCurrentContext());
    std::vector<float> samples(48000 * 2);
    render(device, samples.data(), 48000);
    manager.updateStreams();
    render(device, samples.data(), 48000);
    manager.updateStreams();
    EXPECT_EQ(stream->getPlaybackState(), "buffering");
    EXPECT_GT(stream->getUnderruns(), 0u);
    gate.release();
    stream->prepareSimulation();
    EXPECT_EQ(stream->getPlaybackState(), "playing") << stream->getError();
    EXPECT_LT(stream->getPositionSeconds(), stream->getDurationSeconds());
}

TEST_F(StreamOpenALTest, SeekAfterStopStartsAtSeekPosition){
    auto& file = makeFile(48000);
    auto stream = create(file.path);
    prime(stream);
    stream->stop();
    stream->seekSeconds(0.5);
    prime(stream);
    EXPECT_NEAR(stream->getPositionSeconds(), 0.5, 0.00001);
}

TEST(AudioSamples, StreamMatchesStaticPlaybackAcrossBufferBoundary){
    AudioTestFile file(12000);
    auto render = [&](AudioSourceType type){
        LoopbackManager manager;
        manager.open();
        auto source = manager.createAudioSource(file.path, type);
        source->play();
        manager.simulationTick(5);
        auto samples = *manager.simulationSamples();
        manager.shutdown();
        JobDispatcher::shutdown();
        return samples;
    };
    const auto buffered = render(AudioSourceType::Buffer);
    const auto streamed = render(AudioSourceType::Stream);
    ASSERT_EQ(buffered.size(), streamed.size());
    double maxDifference = 0;
    for(size_t i = 0; i < buffered.size(); ++i)
        maxDifference = std::max(maxDifference, std::abs(double(buffered[i]) - streamed[i]));
    EXPECT_LT(maxDifference, 0.000001);
}
