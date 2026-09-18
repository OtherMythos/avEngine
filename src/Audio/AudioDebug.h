#pragma once

#ifdef DEBUG_SERVER

#include "OgreVector3.h"
#include "System/EnginePrerequisites.h"
#include <chrono>
#include <deque>
#include <map>
#include <string>
#include <vector>

namespace AV{
    class AudioSource;
    class AudioBuffer;

    struct AudioSourceSnapshot{
        std::string state = "unavailable";
        std::string reason = "audio backend unavailable";
        float offset = 0, gain = 0, pitch = 0;
        float rolloff = 0, referenceDistance = 0, maxDistance = 0;
        float minGain = 0, maxGain = 0;
        float coneInnerAngle = 0, coneOuterAngle = 0, coneOuterGain = 0;
        bool looping = false, relative = false;
        Ogre::Vector3 position = Ogre::Vector3::ZERO;
        Ogre::Vector3 velocity = Ogre::Vector3::ZERO;
        Ogre::Vector3 direction = Ogre::Vector3::ZERO;
    };

    struct AudioManagerSnapshot{
        std::string backend = "null", device, setupError;
        bool available = false;
        float gain = 0;
        int distanceModel = 0;
        Ogre::Vector3 position = Ogre::Vector3::ZERO;
        Ogre::Vector3 velocity = Ogre::Vector3::ZERO;
        Ogre::Vector3 forward = Ogre::Vector3::ZERO;
        Ogre::Vector3 up = Ogre::Vector3::ZERO;
    };

    struct AudioBufferDebugInfo{
        std::string path, lastAttemptPath, loadError;
        int channels = 0, sampleRate = 0;
        uint64 frames = 0, bytes = 0;
        std::vector<double> peak, rms;
        void analyse(const short* samples, uint64 frameCount, int channelCount, int rate);
    };

    struct AudioDebugEvent{
        uint64 sequence = 0, frame = 0, sourceId = 0, bufferId = 0;
        double timeSeconds = 0;
        std::string type, path, detail;
    };

    struct AudioEventPage{
        std::vector<AudioDebugEvent> events;
        uint64 nextAfter = 0, latestSequence = 0;
        bool historyGap = false, truncated = false;
    };

    //Main-thread only. Registries observe lifetimes without owning the objects.
    class AudioDebugState{
    public:
        bool enabled = false;
        uint64 frame = 0;
        std::map<uint64, AudioSource*> sources;
        std::map<uint64, AudioBuffer*> buffers;

        uint64 addSource(AudioSource* source);
        uint64 addBuffer(AudioBuffer* buffer);
        void removeSource(uint64 id);
        void removeBuffer(uint64 id);
        void record(const std::string& type, uint64 sourceId = 0, uint64 bufferId = 0,
                    const std::string& path = "", const std::string& detail = "");
        void observe(AudioSource* source);
        void update(uint64 frameNumber);
        AudioEventPage events(uint64 after, bool hasAfter, uint64 sourceId, size_t max) const;
        uint64 latestSequence() const { return mSequence; }
        double timeSeconds() const;

    private:
        uint64 mSequence = 0;
        std::deque<AudioDebugEvent> mEvents;
        std::map<uint64, std::string> mObservedStates;
        std::chrono::steady_clock::time_point mStart = std::chrono::steady_clock::now();
    };
}
#endif
