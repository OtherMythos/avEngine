#ifdef DEBUG_SERVER
#include "AudioDebug.h"
#include "AudioSource.h"
#include "AudioBuffer.h"
#include <algorithm>
#include <atomic>
#include <cmath>

namespace AV{
    namespace{
        std::atomic<uint64_t> nextAudioId{1};
    }

    void AudioBufferDebugInfo::analyse(const short* samples, uint64_t frameCount, int channelCount, int rate){
        frames = frameCount;
        channels = channelCount;
        sampleRate = rate;
        bytes = frames * channels * sizeof(short);
        peak.assign(channels, 0);
        rms.assign(channels, 0);
        for(uint64_t i = 0; i < frames; ++i){
            for(int c = 0; c < channels; ++c){
                const double value = samples[i * channels + c] / 32768.0;
                peak[c] = std::max(peak[c], std::abs(value));
                rms[c] += value * value;
            }
        }
        for(double& value : rms) value = frames ? std::sqrt(value / frames) : 0;
    }

    double AudioDebugState::timeSeconds() const{
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - mStart).count();
    }

    uint64_t AudioDebugState::addSource(AudioSource* source){
        if(!enabled) return 0;
        const uint64_t id = nextAudioId++;
        sources.emplace(id, source);
        record("sourceCreated", id);
        return id;
    }

    uint64_t AudioDebugState::addBuffer(AudioBuffer* buffer){
        if(!enabled) return 0;
        const uint64_t id = nextAudioId++;
        buffers.emplace(id, buffer);
        record("bufferCreated", 0, id);
        return id;
    }

    void AudioDebugState::removeSource(uint64_t id){
        auto it = sources.find(id);
        if(it == sources.end()) return;
        auto buffer = it->second->getAudioBuffer();
        record("sourceDestroyed", id, buffer ? buffer->getDebugId() : 0,
               buffer ? buffer->getDebugInfo().path : "");
        sources.erase(it);
        mObservedStates.erase(id);
    }

    void AudioDebugState::removeBuffer(uint64_t id){
        auto it = buffers.find(id);
        if(it == buffers.end()) return;
        record("bufferDestroyed", 0, id, it->second->getDebugInfo().path);
        buffers.erase(it);
    }

    void AudioDebugState::record(const std::string& type, uint64_t sourceId, uint64_t bufferId,
                                 const std::string& path, const std::string& detail){
        if(!enabled) return;
        AudioDebugEvent event;
        event.sequence = ++mSequence;
        event.frame = frame;
        event.timeSeconds = timeSeconds();
        event.type = type;
        event.sourceId = sourceId;
        event.bufferId = bufferId;
        event.path = path;
        event.detail = detail;
        if(mEvents.size() == 1024) mEvents.pop_front();
        mEvents.push_back(std::move(event));
    }

    void AudioDebugState::observe(AudioSource* source){
        if(!enabled || !source->getDebugId()) return;
        const auto current = source->debugPlaybackState();
        auto& previous = mObservedStates[source->getDebugId()];
        if(previous != current){
            auto buffer = source->getAudioBuffer();
            record("stateObserved", source->getDebugId(), buffer ? buffer->getDebugId() : 0,
                   buffer ? buffer->getDebugInfo().path : "", current);
            previous = current;
        }
    }

    void AudioDebugState::update(uint64_t frameNumber){
        frame = frameNumber;
        for(auto& entry : sources){
            auto it = mObservedStates.find(entry.first);
            if(it == mObservedStates.end() || it->second == "playing") observe(entry.second);
        }
    }

    AudioEventPage AudioDebugState::events(uint64_t after, bool hasAfter, uint64_t sourceId, size_t max) const{
        AudioEventPage page;
        page.latestSequence = mSequence;
        page.nextAfter = after;
        page.historyGap = hasAfter && !mEvents.empty() && after < mEvents.front().sequence - 1;
        if(!hasAfter){
            size_t matches = 0;
            after = mSequence;
            for(auto it = mEvents.rbegin(); it != mEvents.rend(); ++it){
                if(sourceId && it->sourceId != sourceId) continue;
                after = it->sequence - 1;
                if(++matches == max) break;
            }
            page.nextAfter = after;
        }
        for(const auto& event : mEvents){
            if(event.sequence <= after) continue;
            if(!sourceId || event.sourceId == sourceId){
                if(page.events.size() == max){
                    page.truncated = true;
                    break;
                }
                page.events.push_back(event);
            }
            page.nextAfter = event.sequence;
        }
        if(!page.truncated) page.nextAfter = std::max(page.nextAfter, mSequence);
        return page;
    }
}
#endif
