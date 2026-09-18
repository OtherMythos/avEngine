#ifdef DEBUG_SERVER
#include <gtest/gtest.h>
#include <cmath>
#include <limits>
#include "Audio/AudioManager.h"
#include "Audio/AudioSource.h"
#include "Audio/AudioBuffer.h"
#include "System/DebugServer/Inspection/AudioInspector.h"
#include "System/DebugServer/Inspection/DebugJsonUtil.h"

namespace AV{
    namespace{
        class InspectableBuffer : public AudioBuffer{
        public:
            explicit InspectableBuffer(AudioManager* manager) : AudioBuffer(manager) {}
            void samples(const short* data, uint64 frames, int channels){
                mDebugInfo.path = "/sounds/example.wav";
                mDebugInfo.analyse(data, frames, channels, 48000);
                mBufferReady = true;
            }
            void fail(){
                mDebugInfo.lastAttemptPath = "/sounds/missing.wav";
                debugLoadFailure("file missing");
            }
        };

        class InspectableSource : public AudioSource{
        public:
            explicit InspectableSource(AudioManager* manager) : AudioSource(manager) {}
            AudioSourceSnapshot snapshot;
            AudioSourceSnapshot debugSnapshot() const override { return snapshot; }
            void command(const char* name){ debugCommand(name); }
        };

        class InspectableManager : public AudioManager{
        public:
            AudioManagerSnapshot snapshot;
            AudioManagerSnapshot debugSnapshot() const override { return snapshot; }
        };

        bool hasDiagnostic(const rapidjson::Document& doc, const char* code){
            for(const auto& value : doc["diagnostics"].GetArray()){
                if(std::string(value["code"].GetString()) == code) return true;
            }
            return false;
        }
    }

    TEST(AudioDebug, RegistryDoesNotRetainSourcesOrSharedBuffers){
        AudioManager manager;
        manager.debugState().enabled = true;
        uint64 oldId;
        {
            auto buffer = manager.createAudioBuffer();
            auto first = manager.createAudioSourceFromBuffer(buffer);
            oldId = first->getDebugId();
            auto second = manager.createAudioSourceFromBuffer(buffer);
            ASSERT_EQ(manager.debugState().sources.size(), 2u);
            buffer.reset();
            first.reset();
            EXPECT_EQ(manager.debugState().sources.count(oldId), 0u);
            EXPECT_EQ(manager.debugState().buffers.size(), 1u);
            second.reset();
            EXPECT_TRUE(manager.debugState().buffers.empty());
        }
        auto next = manager.createAudioSourceFromBuffer(nullptr);
        EXPECT_GT(next->getDebugId(), oldId);
        EXPECT_EQ(manager.getNumAudioBuffers(), 0);
        rapidjson::Document doc;
        int status = 200;
        AudioInspector::write(doc, status, manager, "source", oldId, AudioQuery(), false);
        EXPECT_EQ(status, 404);
    }

    TEST(AudioDebug, DisabledCollectionHasNoRegistryOrHistory){
        AudioManager manager;
        auto source = manager.createAudioSource("unused");
        manager.debugState().record("play");
        EXPECT_EQ(source->getDebugId(), 0u);
        EXPECT_TRUE(manager.debugState().sources.empty());
        EXPECT_TRUE(manager.debugState().buffers.empty());
        EXPECT_EQ(manager.debugState().latestSequence(), 0u);
    }

    TEST(AudioDebug, EventHistoryIsBoundedNonConsumingAndDetectsGaps){
        AudioDebugState state;
        state.enabled = true;
        for(int i = 0; i < 1030; ++i) state.record("play", 5);
        auto page = state.events(0, true, 0, 50);
        EXPECT_TRUE(page.historyGap);
        EXPECT_TRUE(page.truncated);
        EXPECT_EQ(page.events.front().sequence, 7u);
        EXPECT_EQ(page.events.size(), 50u);
        EXPECT_EQ(page.nextAfter, 56u);
        EXPECT_EQ(state.events(0, true, 0, 50).nextAfter, page.nextAfter);
        auto latest = state.events(0, false, 0, 50);
        EXPECT_EQ(latest.events.front().sequence, 981u);
        EXPECT_EQ(latest.events.back().sequence, 1030u);
        EXPECT_FALSE(latest.historyGap);
        EXPECT_FALSE(latest.truncated);
        EXPECT_FALSE(state.events(6, true, 0, 50).historyGap);
    }

    TEST(AudioDebug, FilteredEventCursorAdvancesOverUnmatchedEvents){
        AudioDebugState state;
        state.enabled = true;
        state.record("play", 1);
        state.record("pause", 2);
        state.record("stop", 1);
        state.record("play", 2);
        auto page = state.events(0, true, 1, 1);
        EXPECT_EQ(page.nextAfter, 2u);
        EXPECT_TRUE(page.truncated);
        page = state.events(page.nextAfter, true, 1, 1);
        ASSERT_EQ(page.events.size(), 1u);
        EXPECT_EQ(page.events[0].sequence, 3u);
        EXPECT_EQ(page.nextAfter, 4u);
        EXPECT_FALSE(page.truncated);
        EXPECT_EQ(state.events(0, true, 99, 1).nextAfter, 4u);
    }

    TEST(AudioDebug, ShortLivedCommandsAndPathsSurviveDestruction){
        AudioManager manager;
        manager.debugState().enabled = true;
        uint64 id;
        {
            auto buffer = std::make_shared<InspectableBuffer>(&manager);
            short samples[] = {0};
            buffer->samples(samples, 1, 1);
            InspectableSource source(&manager);
            source.setAudioBuffer(buffer);
            id = source.getDebugId();
            source.command("play");
        }
        const auto page = manager.debugState().events(0, true, id, 50);
        ASSERT_EQ(page.events.size(), 4u);
        EXPECT_EQ(page.events[2].type, "play");
        EXPECT_EQ(page.events[3].type, "sourceDestroyed");
        EXPECT_EQ(page.events[3].path, "/sounds/example.wav");
    }

    TEST(AudioDebug, ObservesNaturalCompletionOnce){
        AudioManager manager;
        manager.debugState().enabled = true;
        InspectableSource source(&manager);
        source.snapshot.state = "playing";
        manager.debugState().observe(&source);
        auto sequence = manager.debugState().latestSequence();
        source.snapshot.state = "stopped";
        manager.debugState().update(10);
        manager.debugState().update(11);
        const auto page = manager.debugState().events(sequence, true, 0, 50);
        ASSERT_EQ(page.events.size(), 1u);
        EXPECT_EQ(page.events[0].type, "stateObserved");
        EXPECT_EQ(page.events[0].detail, "stopped");
        EXPECT_EQ(page.events[0].frame, 10u);
    }

    TEST(AudioDebug, DecodedStatisticsArePerChannelAndHandleNegativeFullScale){
        AudioBufferDebugInfo info;
        short samples[] = {-32768, 16384, 0, -16384};
        info.analyse(samples, 2, 2, 48000);
        EXPECT_DOUBLE_EQ(info.peak[0], 1);
        EXPECT_DOUBLE_EQ(info.peak[1], 0.5);
        EXPECT_NEAR(info.rms[0], std::sqrt(0.5), 1e-12);
        EXPECT_DOUBLE_EQ(info.rms[1], 0.5);
        EXPECT_EQ(info.bytes, 8u);
        short silence[] = {0, 0};
        info.analyse(silence, 2, 1, 24000);
        EXPECT_DOUBLE_EQ(info.peak[0], 0);
        EXPECT_DOUBLE_EQ(info.rms[0], 0);
    }

    TEST(AudioInspector, ListPaginationAndSharedBufferReferences){
        AudioManager manager;
        manager.debugState().enabled = true;
        auto buffer = std::make_shared<InspectableBuffer>(&manager);
        short samples[] = {1};
        buffer->samples(samples, 1, 1);
        auto first = manager.createAudioSourceFromBuffer(buffer);
        auto second = manager.createAudioSourceFromBuffer(buffer);
        AudioQuery query;
        query.max = 1;
        query.path = "example";
        int status = 200;
        rapidjson::Document doc;
        AudioInspector::write(doc, status, manager, "sources", 0, query, false);
        EXPECT_EQ(doc["total"].GetUint64(), 2u);
        EXPECT_TRUE(doc["truncated"].GetBool());
        EXPECT_EQ(doc["sources"][0]["id"].GetUint64(), first->getDebugId());
        query.after = doc["nextAfter"].GetUint64();
        AudioInspector::write(doc, status, manager, "sources", 0, query, false);
        EXPECT_FALSE(doc["truncated"].GetBool());
        EXPECT_EQ(doc["sources"][0]["id"].GetUint64(), second->getDebugId());
        AudioInspector::write(doc, status, manager, "buffer", buffer->getDebugId(), AudioQuery(), false);
        EXPECT_EQ(doc["sourceCount"].GetUint(), 2u);
        EXPECT_STREQ(doc["sampleStats"]["scope"].GetString(), "wholeDecodedAsset");
        query.state = "playing";
        AudioInspector::write(doc, status, manager, "sources", 0, query, false);
        EXPECT_EQ(doc["total"].GetUint64(), 0u);
    }

    TEST(AudioInspector, UnavailableValuesAreNullWithDiagnostics){
        AudioManager manager;
        manager.debugState().enabled = true;
        auto source = manager.createAudioSourceFromBuffer(nullptr);
        rapidjson::Document doc;
        int status = 200;
        AudioInspector::write(doc, status, manager, "source", source->getDebugId(), AudioQuery(), true);
        EXPECT_TRUE(doc["gain"].IsNull());
        EXPECT_TRUE(doc["position"].IsNull());
        EXPECT_TRUE(hasDiagnostic(doc, "audioDisabled"));
        EXPECT_TRUE(hasDiagnostic(doc, "missingBuffer"));
        EXPECT_TRUE(hasDiagnostic(doc, "backendUnavailable"));
        AudioInspector::write(doc, status, manager, "", 0, AudioQuery(), true);
        EXPECT_TRUE(doc["masterGain"].IsNull());
        EXPECT_FALSE(doc["outputMeasured"].GetBool());
    }

    TEST(AudioInspector, DiagnosesSilenceWithoutInferringDistanceCutoff){
        InspectableManager manager;
        manager.debugState().enabled = true;
        manager.snapshot.available = true;
        auto buffer = std::make_shared<InspectableBuffer>(&manager);
        short samples[] = {0, 0};
        buffer->samples(samples, 2, 1);
        buffer->fail();
        InspectableSource source(&manager);
        source.setAudioBuffer(buffer);
        source.snapshot.state = "playing";
        source.snapshot.position = Ogre::Vector3(100, 0, 0);
        source.snapshot.maxDistance = 10;
        rapidjson::Document doc;
        int status = 200;
        AudioInspector::write(doc, status, manager, "source", source.getDebugId(), AudioQuery(), false);
        EXPECT_TRUE(hasDiagnostic(doc, "silentAsset"));
        EXPECT_TRUE(hasDiagnostic(doc, "zeroMasterGain"));
        EXPECT_TRUE(hasDiagnostic(doc, "zeroSourceGain"));
        EXPECT_TRUE(hasDiagnostic(doc, "loadFailed"));
        EXPECT_EQ(doc["diagnostics"].Size(), 4u);
        EXPECT_DOUBLE_EQ(doc["distanceFromListener"].GetDouble(), 100);
        EXPECT_TRUE(doc["buffer"]["ready"].GetBool());
        EXPECT_STREQ(doc["buffer"]["path"].GetString(), "/sounds/example.wav");
        EXPECT_FALSE(DebugJsonUtil::toString(doc).empty());
    }

    TEST(AudioInspector, RejectsMalformedOrOverflowingIds){
        uint64 value = 0;
        for(const char* text : {"", "-1", "+1", "1.0", "1x", " 1", "18446744073709551616"}){
            EXPECT_FALSE(AudioInspector::parseUnsigned(text, value)) << text;
        }
        EXPECT_TRUE(AudioInspector::parseUnsigned("18446744073709551615", value));
        EXPECT_EQ(value, std::numeric_limits<uint64>::max());
    }
}
#endif
