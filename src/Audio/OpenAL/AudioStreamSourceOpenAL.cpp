#include "AudioStreamSourceOpenAL.h"

#include "Audio/AudioManager.h"
#include "Logger/Log.h"
#include <algorithm>
#include <cmath>

namespace AV{
    AudioStreamSourceOpenAL::AudioStreamSourceOpenAL(AudioManager* manager, const std::string& path)
        : AudioSourceOpenAL(manager), mPath(path), mStream(std::make_shared<AudioStream>(path)){
        mSourcePath = path;
        if(!mSource){ mError = "audio backend unavailable"; return; }
        alGenBuffers(static_cast<ALsizei>(mBuffers.size()), mBuffers.data());
        if(alGetError() != AL_NO_ERROR){ mError = "Could not allocate streaming buffers"; return; }
        for(auto buffer : mBuffers) mFree.push_back(buffer);
        mStream->reset(0, false);
    }

    AudioStreamSourceOpenAL::~AudioStreamSourceOpenAL(){
        releaseBackend();
    }

    void AudioStreamSourceOpenAL::releaseBackend(){
        mStream->cancel();
        flush();
        if(mSource && mManager && mManager->isSetup()){
            for(auto& buffer : mBuffers){
                if(buffer) alDeleteBuffers(1, &buffer);
                buffer = 0;
            }
        }
        AudioSourceOpenAL::releaseBackend();
    }

    void AudioStreamSourceOpenAL::flush(){
        if(mSource && mManager && mManager->isSetup()){
            alSourceStop(mSource);
            alSourcei(mSource, AL_BUFFER, 0);
            alSourceRewind(mSource);
        }
        mQueue.clear();
        mFree.clear();
        for(auto buffer : mBuffers) if(buffer) mFree.push_back(buffer);
        mStarted = false;
        mStarved = false;
    }

    void AudioStreamSourceOpenAL::reset(double seconds, bool active){
        flush();
        mPosition = seconds;
        mEnded = false;
        mError.clear();
        mStream->reset(seconds, mLooping, active);
        mStatus = mStream->status();
    }

    void AudioStreamSourceOpenAL::setAudioBuffer(AudioBufferPtr){
#ifdef DEBUG_SERVER
        debugCommand("operationFailed", "A stream cannot be assigned a static audio buffer");
#endif
    }

    void AudioStreamSourceOpenAL::play(){
        if(!mSource) return;
        if(mRewindOnPlay || mEnded) reset(0, true);
        mRewindOnPlay = false;
        mIntent = Intent::Playing;
#ifdef DEBUG_SERVER
        debugCommand("play");
#endif
        update();
    }

    void AudioStreamSourceOpenAL::pause(){
        if(!mSource) return;
        mIntent = Intent::Paused;
        alSourcePause(mSource);
#ifdef DEBUG_SERVER
        debugCommand("pause");
#endif
    }

    void AudioStreamSourceOpenAL::stop(){
        if(!mSource) return;
        mIntent = Intent::Stopped;
        mRewindOnPlay = true;
        reset(0, false);
#ifdef DEBUG_SERVER
        debugCommand("stop");
#endif
    }

    void AudioStreamSourceOpenAL::setLooping(bool looping){
        if(looping == mLooping || !mSource) return;
        const double position = getPositionSeconds();
        mLooping = looping;
        reset(position, mIntent != Intent::Stopped);
    }

    void AudioStreamSourceOpenAL::seekSeconds(float seconds){
        if(!mSource || !std::isfinite(seconds) || seconds < 0) return;
        mRewindOnPlay = false;
        const double duration = getDurationSeconds();
        reset(duration < 0 ? seconds : std::min<double>(duration, seconds), true);
#ifdef DEBUG_SERVER
        debugCommand("seek", std::to_string(seconds));
#endif
    }

    void AudioStreamSourceOpenAL::fail(const std::string& error){
        mError = error;
        AV_ERROR("Audio stream {}: {}", mPath, error);
        flush();
        mStream->cancel();
#ifdef DEBUG_SERVER
        debugCommand("streamFailed", error);
#endif
    }

    void AudioStreamSourceOpenAL::update(){
        if(!mSource || !mError.empty()) return;
        mStream->schedule();
        mStatus = mStream->status();
        if(!mStatus.error.empty()){ fail(mStatus.error); return; }
        ALint processed = 0;
        alGetSourcei(mSource, AL_BUFFERS_PROCESSED, &processed);
        while(processed-- > 0 && !mQueue.empty()){
            const auto front = mQueue.front();
            ALuint buffer = 0;
            alSourceUnqueueBuffers(mSource, 1, &buffer);
            mQueue.pop_front();
            mFree.push_back(buffer);
            mPosition = double(front.firstFrame + front.frames) / mStatus.rate;
            if(mLooping && mStatus.frames > 0) mPosition = std::fmod(mPosition, getDurationSeconds());
        }
        ALint state = AL_INITIAL;
        alGetSourcei(mSource, AL_SOURCE_STATE, &state);
        if(mIntent == Intent::Playing && mStarted && state == AL_STOPPED && mQueue.empty() &&
           !(mStatus.eof && mStatus.readyFrames == 0) && !mStarved){
            ++mUnderruns;
            mStarved = true;
#ifdef DEBUG_SERVER
            debugCommand("streamUnderrun");
#endif
        }
        while(!mFree.empty()){
            size_t slot;
            const auto* block = mStream->acquire(slot);
            if(!block) break;
            mStatus = mStream->status();
            const ALuint buffer = mFree.front();
            mFree.pop_front();
            alBufferData(buffer, mStatus.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
                block->samples.data(), static_cast<ALsizei>(block->frames * mStatus.channels * sizeof(short)), mStatus.rate);
            alSourceQueueBuffers(mSource, 1, &buffer);
            mQueue.push_back({buffer, block->frames, block->firstFrame});
            mStream->release(slot);
        }
        const auto error = alGetError();
        if(error != AL_NO_ERROR){ fail(std::string("OpenAL streaming: ") + alGetString(error)); return; }
        mStatus = mStream->status();
        if(mIntent == Intent::Playing){
            if(!mQueue.empty() && (mStarted || mQueue.size() >= 2 || mStatus.eof)){
                if(state != AL_PLAYING) alSourcePlay(mSource);
                mStarted = true;
                mStarved = false;
            }else if(mQueue.empty() && mStatus.eof && !mStatus.readyFrames && !mStatus.pending){
                mEnded = true;
                mPosition = getDurationSeconds();
            }
        }
    }

    void AudioStreamSourceOpenAL::prepareSimulation(){
        if(mIntent != Intent::Playing || !mSource || !mError.empty()) return;
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while(true){
            update();
            if(!mError.empty() || mEnded) return;
            //Mix only against a full queue, so what the mixer can see never depends on decoder timing.
            //Pitch, doppler and resampler lookahead are all far smaller than the queued blocks.
            if((mStarted && mQueue.size() == mBuffers.size()) ||
                (mStatus.eof && !mStatus.readyFrames && !mStatus.pending)) return;
            if(std::chrono::steady_clock::now() >= deadline){
                fail("Timed out waiting for streamed simulation audio; worker pool may be saturated");
                return;
            }
            mStream->schedule();
            mStream->wait(std::chrono::milliseconds(20));
        }
    }

    std::string AudioStreamSourceOpenAL::getPlaybackState() const{
        if(!mSource) return "unavailable";
        if(!getError().empty()) return "failed";
        if(mIntent == Intent::Stopped) return "stopped";
        if(mIntent == Intent::Paused) return "paused";
        if(mEnded) return "ended";
        if(!mStatus.rate) return "opening";
        if(mIntent == Intent::Ready) return mQueue.empty() ? "opening" : "ready";
        ALint state;
        alGetSourcei(mSource, AL_SOURCE_STATE, &state);
        return state == AL_PLAYING ? "playing" : "buffering";
    }

    double AudioStreamSourceOpenAL::getDurationSeconds() const{
        return mStatus.rate ? double(mStatus.frames) / mStatus.rate : -1;
    }

    double AudioStreamSourceOpenAL::getPositionSeconds() const{
        if(mQueue.empty() || !mSource || !mStatus.rate) return mPosition;
        ALfloat offset = 0;
        alGetSourcef(mSource, AL_SEC_OFFSET, &offset);
        double position = double(mQueue.front().firstFrame) / mStatus.rate + offset;
        if(mLooping && mStatus.frames > 0) position = std::fmod(position, getDurationSeconds());
        return position;
    }

    double AudioStreamSourceOpenAL::getBufferedSeconds() const{
        if(!mSource || !mStatus.rate) return 0;
        size_t frames = 0;
        for(const auto& buffer : mQueue) frames += buffer.frames;
        ALfloat offset = 0;
        alGetSourcef(mSource, AL_SEC_OFFSET, &offset);
        return std::max(0.0, double(frames) / mStatus.rate - offset);
    }

    double AudioStreamSourceOpenAL::getDecodedSeconds() const{
        const auto status = mStream->status();
        return status.rate ? double(status.readyFrames) / status.rate : 0;
    }

    std::string AudioStreamSourceOpenAL::getError() const{
        return mError.empty() ? mStream->status().error : mError;
    }

#ifdef DEBUG_SERVER
    AudioSourceSnapshot AudioStreamSourceOpenAL::debugSnapshot() const{
        auto out = AudioSourceOpenAL::debugSnapshot();
        out.state = getPlaybackState();
        out.offset = static_cast<float>(getPositionSeconds());
        out.looping = mLooping;
        return out;
    }
#endif
}
