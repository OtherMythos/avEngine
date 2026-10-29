#include "AudioStream.h"

#include "Threading/JobDispatcher.h"
#include "Threading/Jobs/Job.h"
#include "sndfile.h"
#include <algorithm>
#include <cmath>

namespace AV{
    class AudioStream::DecodeJob : public Job{
    public:
        explicit DecodeJob(std::shared_ptr<AudioStream> stream) : mStream(std::move(stream)) {}
        ~DecodeJob() override { mStream->jobFinished(); }
        void process() override { mStream->decode(); }
        void finish() override {}
    private:
        std::shared_ptr<AudioStream> mStream;
    };

    AudioStream::AudioStream(const std::string& path) : mPath(path){
        for(size_t i = 0; i < BlockCount; ++i) mFree.push_back(i);
    }

    AudioStream::~AudioStream(){
        if(mFile) sf_close(mFile);
    }

    void AudioStream::reset(double seconds, bool looping, bool active){
        {
            std::lock_guard<std::mutex> lock(mMutex);
            if(mCancelled) return;
            ++mGeneration;
            mSeekSeconds = seconds;
            mLooping = looping;
            mActive = active;
            mStatus.eof = false;
            mStatus.error.clear();
            while(!mReady.empty()){
                mFree.push_back(mReady.front());
                mReady.pop_front();
            }
        }
        schedule();
    }

    void AudioStream::cancel(){
        std::lock_guard<std::mutex> lock(mMutex);
        ++mGeneration;
        mCancelled = true;
        mActive = false;
        mChanged.notify_all();
    }

    void AudioStream::schedule(){
        {
            std::lock_guard<std::mutex> lock(mMutex);
            if(mCancelled || !mActive || mPending || mFree.empty() || mStatus.eof || !mStatus.error.empty()) return;
            mPending = true;
        }
        JobDispatcher::dispatchJob(new DecodeJob(shared_from_this()), JobDispatcher::Priority::High);
    }

    void AudioStream::jobFinished(){
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mPending = false;
        }
        mChanged.notify_all();
        //The owner schedules the next small job. Never recursively dispatch from
        //a job destructor: queued jobs are also destroyed during pool shutdown.
    }

    AudioStream::Status AudioStream::status() const{
        std::lock_guard<std::mutex> lock(mMutex);
        auto result = mStatus;
        result.pending = mPending;
        for(size_t slot : mReady) result.readyFrames += mBlocks[slot].frames;
        return result;
    }

    const AudioStream::Block* AudioStream::acquire(size_t& slot){
        std::lock_guard<std::mutex> lock(mMutex);
        if(mReady.empty()) return nullptr;
        slot = mReady.front();
        mReady.pop_front();
        return &mBlocks[slot];
    }

    void AudioStream::release(size_t slot){
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mFree.push_back(slot);
        }
        schedule();
    }

    bool AudioStream::wait(std::chrono::milliseconds timeout){
        std::unique_lock<std::mutex> lock(mMutex);
        return mChanged.wait_for(lock, timeout, [this]{ return !mPending || mCancelled; });
    }

    void AudioStream::decode(){
        size_t slot;
        uint64_t generation;
        double seconds;
        bool looping;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            if(mCancelled || !mActive || mFree.empty()) return;
            slot = mFree.front();
            mFree.pop_front();
            generation = mGeneration;
            seconds = mSeekSeconds;
            looping = mLooping;
        }

        std::string error;
        if(!mFile){
            SF_INFO info = {};
            mFile = sf_open(mPath.c_str(), SFM_READ, &info);
            if(!mFile) error = "Could not open audio file: " + mPath;
            else if(info.channels < 1 || info.channels > 2 || info.samplerate < 1 || info.frames < 1 || !info.seekable){
                error = "Streaming requires a non-empty seekable mono or stereo audio file";
                sf_close(mFile);
                mFile = nullptr;
            }else{
                mRate = info.samplerate;
                mChannels = info.channels;
                mFrames = info.frames;
            }
        }
        if(error.empty() && generation != mDecoderGeneration){
            //Clamp before converting so huge script offsets cannot overflow.
            const double frame = std::max(0.0, std::min(seconds * mRate, double(mFrames)));
            mCursor = std::isfinite(frame) ? static_cast<int64_t>(frame) : 0;
            if(looping && mCursor == mFrames) mCursor = 0;
            if(sf_seek(mFile, mCursor, SEEK_SET) != mCursor) error = "Could not seek audio file";
            mDecoderGeneration = generation;
        }

        auto& block = mBlocks[slot];
        block.frames = 0;
        block.firstFrame = mCursor;
        bool eof = false;
        while(error.empty() && block.frames < BlockFrames){
            if(mCursor >= mFrames){
                if(!looping){ eof = true; break; }
                if(sf_seek(mFile, 0, SEEK_SET) != 0){ error = "Could not loop audio file"; break; }
                mCursor = 0;
            }
            const size_t count = std::min<int64_t>(BlockFrames - block.frames, mFrames - mCursor);
            const auto read = sf_readf_short(mFile, block.samples.data() + block.frames * mChannels, count);
            if(read != static_cast<sf_count_t>(count) || sf_error(mFile) != SF_ERR_NO_ERROR){
                error = "Could not decode audio samples";
                break;
            }
            block.frames += static_cast<size_t>(read);
            mCursor += read;
        }
        if(error.empty() && !looping && mCursor == mFrames) eof = true;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            if(generation == mGeneration && !mCancelled){
                mStatus.rate = mRate;
                mStatus.channels = mChannels;
                mStatus.frames = mFrames;
                mStatus.error = error;
                mStatus.eof = eof;
                if(error.empty() && block.frames) mReady.push_back(slot);
                else mFree.push_back(slot);
            }else mFree.push_back(slot);
        }
    }
}
