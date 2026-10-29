#include <gtest/gtest.h>
#include "Audio/AudioStream.h"
#include "Threading/JobDispatcher.h"
#include "AudioTestFiles.h"

using namespace AV;

namespace{
    class AudioStreamTest : public testing::Test{
    protected:
        std::shared_ptr<AudioStream> stream;
        void TearDown() override{
            if(stream) stream->cancel();
            JobDispatcher::shutdown();
            stream.reset();
        }
        void open(const std::string& path, bool loop = false, double seconds = 0){
            stream = std::make_shared<AudioStream>(path);
            stream->reset(seconds, loop);
        }
        bool finish(){ return stream->wait(std::chrono::milliseconds(1000)); }
        const AudioStream::Block* next(size_t& slot){
            stream->schedule();
            if(!finish()) return nullptr;
            return stream->acquire(slot);
        }
    };
}

TEST_F(AudioStreamTest, ReadsStereoSamplesAndMetadata){
    AudioTestFile file;
    open(file.path);
    size_t slot;
    auto* block = next(slot);
    ASSERT_NE(block, nullptr);
    EXPECT_EQ(block->frames, 4800u);
    EXPECT_EQ(block->firstFrame, 0);
    EXPECT_TRUE(std::equal(file.samples.begin(), file.samples.end(), block->samples.begin()));
    auto status = stream->status();
    EXPECT_EQ(status.rate, 48000);
    EXPECT_EQ(status.channels, 2);
    EXPECT_EQ(status.frames, 4800);
    EXPECT_TRUE(status.eof);
    EXPECT_TRUE(status.error.empty());
    stream->release(slot);
}

TEST_F(AudioStreamTest, ReadsMonoAndDifferentSampleRate){
    AudioTestFile file(128, 1, 22050);
    open(file.path);
    size_t slot;
    auto* block = next(slot);
    ASSERT_NE(block, nullptr);
    EXPECT_TRUE(std::equal(file.samples.begin(), file.samples.end(), block->samples.begin()));
    EXPECT_EQ(stream->status().rate, 22050);
    EXPECT_EQ(stream->status().channels, 1);
    stream->release(slot);
}

TEST_F(AudioStreamTest, FinalPartialBlockIsNotPadded){
    AudioTestFile file(AudioStream::BlockFrames + 13);
    open(file.path);
    size_t slot;
    auto* block = next(slot);
    ASSERT_NE(block, nullptr);
    EXPECT_EQ(block->frames, AudioStream::BlockFrames);
    stream->release(slot);
    block = next(slot);
    ASSERT_NE(block, nullptr);
    EXPECT_EQ(block->frames, 13u);
    EXPECT_EQ(block->firstFrame, AudioStream::BlockFrames);
    EXPECT_TRUE(stream->status().eof);
    stream->release(slot);
    EXPECT_EQ(next(slot), nullptr);
}

TEST_F(AudioStreamTest, ExactBlockEndPublishesEof){
    AudioTestFile file(AudioStream::BlockFrames);
    open(file.path);
    ASSERT_TRUE(finish());
    EXPECT_TRUE(stream->status().eof);
    EXPECT_EQ(stream->status().readyFrames, AudioStream::BlockFrames);
}

TEST_F(AudioStreamTest, RingNeverExceedsCapacity){
    AudioTestFile file(AudioStream::BlockFrames * 8);
    open(file.path);
    for(size_t i = 0; i < AudioStream::BlockCount + 2; ++i){
        stream->schedule();
        ASSERT_TRUE(finish());
    }
    EXPECT_EQ(stream->status().readyFrames, AudioStream::BlockFrames * AudioStream::BlockCount);
    EXPECT_FALSE(stream->status().pending);
    EXPECT_LT(AudioStream::memoryBytes(), 132000u);
    size_t slot;
    const auto* block = stream->acquire(slot);
    ASSERT_NE(block, nullptr);
    EXPECT_EQ(block->firstFrame, 0);
    stream->release(slot);
    ASSERT_TRUE(finish());
    EXPECT_EQ(stream->status().readyFrames, AudioStream::BlockFrames * AudioStream::BlockCount);
}

TEST_F(AudioStreamTest, LoopSeamHasNoPaddingOrDuplicatedFrames){
    AudioTestFile file(127, 2);
    open(file.path, true);
    size_t slot;
    auto* block = next(slot);
    ASSERT_NE(block, nullptr);
    for(size_t i = 0; i < block->frames * 2; ++i) ASSERT_EQ(block->samples[i], file.samples[i % file.samples.size()]);
    EXPECT_FALSE(stream->status().eof);
    stream->release(slot);
}

TEST_F(AudioStreamTest, SeekUsesSampleFrames){
    AudioTestFile file(48000);
    open(file.path, false, 0.25);
    size_t slot;
    auto* block = next(slot);
    ASSERT_NE(block, nullptr);
    EXPECT_EQ(block->firstFrame, 12000);
    EXPECT_EQ(block->samples[0], file.samples[24000]);
    stream->release(slot);
}

TEST_F(AudioStreamTest, SeekPastEndClampsWithoutOverflow){
    AudioTestFile file;
    open(file.path, false, 1e30);
    ASSERT_TRUE(finish());
    EXPECT_TRUE(stream->status().eof);
    EXPECT_EQ(stream->status().readyFrames, 0u);
    EXPECT_TRUE(stream->status().error.empty());
}

TEST_F(AudioStreamTest, ResetDiscardsPreviouslyDecodedBlocks){
    AudioTestFile file(48000);
    open(file.path);
    ASSERT_TRUE(finish());
    stream->reset(0.5, false);
    size_t slot;
    auto* block = next(slot);
    ASSERT_NE(block, nullptr);
    EXPECT_EQ(block->firstFrame, 24000);
    stream->release(slot);
}

TEST_F(AudioStreamTest, InFlightResetCannotPublishOldGeneration){
    AudioTestFile file(48000);
    open(file.path);
    stream->reset(0.75, false);
    ASSERT_TRUE(finish());
    size_t slot;
    auto* block = next(slot);
    ASSERT_NE(block, nullptr);
    EXPECT_EQ(block->firstFrame, 36000);
    stream->release(slot);
}

TEST_F(AudioStreamTest, StoppedStreamDoesNotScheduleMoreWork){
    AudioTestFile file(48000);
    open(file.path);
    stream->reset(0, false, false);
    ASSERT_TRUE(finish());
    stream->schedule();
    EXPECT_FALSE(stream->status().pending);
    EXPECT_EQ(stream->status().readyFrames, 0u);
}

TEST_F(AudioStreamTest, CancellationNeverPublishesPendingData){
    AudioTestFile file;
    open(file.path);
    stream->cancel();
    JobDispatcher::shutdown();
    stream->reset(0, false);
    EXPECT_FALSE(stream->status().pending);
    //A job finished before cancellation may already have published a block;
    //the owner is gone, and cancellation must never start another job.
    EXPECT_EQ(JobDispatcher::activeWorkerCount(), 0u);
}

TEST_F(AudioStreamTest, MissingFileFailsRatherThanEnding){
    open("/does-not-exist/stream.wav");
    ASSERT_TRUE(finish());
    EXPECT_FALSE(stream->status().error.empty());
    EXPECT_FALSE(stream->status().eof);
}

TEST_F(AudioStreamTest, CorruptFileFails){
    AudioTestFile file;
    { std::ofstream out(file.path, std::ios::trunc); out << "not audio"; }
    open(file.path);
    ASSERT_TRUE(finish());
    EXPECT_FALSE(stream->status().error.empty());
}

TEST_F(AudioStreamTest, EmptyFileFails){
    AudioTestFile file(0);
    open(file.path);
    ASSERT_TRUE(finish());
    EXPECT_FALSE(stream->status().error.empty());
}

TEST_F(AudioStreamTest, UnsupportedChannelsFail){
    AudioTestFile file(100, 3);
    open(file.path);
    ASSERT_TRUE(finish());
    EXPECT_FALSE(stream->status().error.empty());
}

#include "AudioPoolGate.h"

TEST_F(AudioStreamTest, SaturatedPoolKeepsOpeningAsynchronousAndCancellationSafe){
    AudioTestFile file;
    AudioPoolGate gate;
    ASSERT_TRUE(gate.wait());
    open(file.path);
    EXPECT_TRUE(stream->status().pending);
    EXPECT_EQ(stream->status().rate, 0);
    stream->reset(0.05, false);
    stream->cancel();
    gate.release();
    JobDispatcher::shutdown();
    EXPECT_FALSE(stream->status().pending);
    EXPECT_EQ(stream->status().readyFrames, 0u);
}

TEST_F(AudioStreamTest, SeekWhileQueuedPublishesOnlyNewPosition){
    AudioTestFile file;
    AudioPoolGate gate;
    ASSERT_TRUE(gate.wait());
    open(file.path);
    stream->reset(0.05, false);
    gate.release();
    size_t slot;
    const auto* block = next(slot);
    ASSERT_NE(block, nullptr);
    EXPECT_EQ(block->firstFrame, 2400);
    stream->release(slot);
}
