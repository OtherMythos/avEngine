#ifdef DEBUG_SERVER
#include "AudioInspector.h"
#include "DebugJsonUtil.h"
#include "Audio/AudioManager.h"
#include "Audio/AudioSource.h"
#include "Audio/AudioBuffer.h"
#include "AL/al.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace AV{
    namespace{
        using Allocator = rapidjson::Document::AllocatorType;
        using Value = rapidjson::Value;

        Value str(const std::string& text, Allocator& a){ return Value(text.c_str(), a); }
        Value number(double value, bool available = true){
            return available && std::isfinite(value) ? Value(value) : Value();
        }
        Value vec(const Ogre::Vector3& value, bool available, Allocator& a){
            if(!available || !std::isfinite(value.x) || !std::isfinite(value.y) || !std::isfinite(value.z)) return Value();
            return DebugJsonUtil::vector3(value, a);
        }
        const char* distanceModel(int model){
            switch(model){
                case AL_NONE: return "none";
                case AL_INVERSE_DISTANCE: return "inverse";
                case AL_INVERSE_DISTANCE_CLAMPED: return "inverseClamped";
                case AL_LINEAR_DISTANCE: return "linear";
                case AL_LINEAR_DISTANCE_CLAMPED: return "linearClamped";
                case AL_EXPONENT_DISTANCE: return "exponent";
                case AL_EXPONENT_DISTANCE_CLAMPED: return "exponentClamped";
                default: return "unknown";
            }
        }

        Value bufferValue(const AudioBuffer& buffer, const AudioDebugState& state, bool detail, Allocator& a){
            const auto& info = buffer.getDebugInfo();
            Value out(rapidjson::kObjectType);
            out.AddMember("id", DebugJsonUtil::uint64Value(buffer.getDebugId()), a);
            out.AddMember("path", str(info.path, a), a);
            out.AddMember("ready", buffer.isReady(), a);
            const bool decoded = info.sampleRate > 0;
            out.AddMember("durationSeconds", number(decoded ? double(info.frames) / info.sampleRate : 0, decoded), a);
            out.AddMember("channels", decoded ? Value(info.channels) : Value(), a);
            out.AddMember("sampleRate", decoded ? Value(info.sampleRate) : Value(), a);
            out.AddMember("decodedBytes", decoded ? DebugJsonUtil::uint64Value(info.bytes) : Value(), a);
            unsigned int references = 0;
            for(const auto& entry : state.sources){
                if(entry.second->getAudioBuffer().get() == &buffer) ++references;
            }
            out.AddMember("sourceCount", references, a);
            if(detail){
                out.AddMember("lastAttemptPath", str(info.lastAttemptPath, a), a);
                out.AddMember("loadError", info.loadError.empty() ? Value() : str(info.loadError, a), a);
                out.AddMember("sampleFrames", decoded ? DebugJsonUtil::uint64Value(info.frames) : Value(), a);
                Value stats(rapidjson::kObjectType), peaks(rapidjson::kArrayType), rms(rapidjson::kArrayType);
                for(double v : info.peak) peaks.PushBack(number(v), a);
                for(double v : info.rms) rms.PushBack(number(v), a);
                stats.AddMember("scope", "wholeDecodedAsset", a);
                stats.AddMember("peak", peaks, a);
                stats.AddMember("rms", rms, a);
                if(decoded) out.AddMember("sampleStats", stats, a);
                else out.AddMember("sampleStats", Value(), a);
            }
            return out;
        }

        Value sourceValue(const AudioSource& source, const AudioSourceSnapshot& snap,
                          const AudioManagerSnapshot& manager, const AudioDebugState& state,
                          bool disabled, bool detail, Allocator& a){
            auto buffer = source.getAudioBuffer();
            bool available = snap.state != "unavailable";
            Value out(rapidjson::kObjectType);
            out.AddMember("id", DebugJsonUtil::uint64Value(source.getDebugId()), a);
            out.AddMember("bufferId", buffer ? DebugJsonUtil::uint64Value(buffer->getDebugId()) : Value(), a);
            out.AddMember("path", buffer ? str(buffer->getDebugInfo().path, a) : Value(), a);
            out.AddMember("state", str(snap.state, a), a);
            out.AddMember("unavailableReason", available ? Value() : str(snap.reason, a), a);
            out.AddMember("offsetSeconds", number(snap.offset, available), a);
            out.AddMember("gain", number(snap.gain, available), a);
            out.AddMember("pitch", number(snap.pitch, available), a);
            out.AddMember("looping", available ? Value(snap.looping) : Value(), a);
            out.AddMember("relative", available ? Value(snap.relative) : Value(), a);
            out.AddMember("position", vec(snap.position, available, a), a);
            if(!detail) return out;
            out.AddMember("velocity", vec(snap.velocity, available, a), a);
            out.AddMember("direction", vec(snap.direction, available, a), a);
            out.AddMember("rolloff", number(snap.rolloff, available), a);
            out.AddMember("referenceDistance", number(snap.referenceDistance, available), a);
            out.AddMember("maxDistance", number(snap.maxDistance, available), a);
            out.AddMember("minGain", number(snap.minGain, available), a);
            out.AddMember("maxGain", number(snap.maxGain, available), a);
            out.AddMember("coneInnerAngle", number(snap.coneInnerAngle, available), a);
            out.AddMember("coneOuterAngle", number(snap.coneOuterAngle, available), a);
            out.AddMember("coneOuterGain", number(snap.coneOuterGain, available), a);
            const double distance = snap.relative ? snap.position.length() : (snap.position - manager.position).length();
            out.AddMember("distanceFromListener", number(distance, available && manager.available), a);
            out.AddMember("buffer", buffer ? bufferValue(*buffer, state, true, a) : Value(), a);
            Value diagnostics(rapidjson::kArrayType);
            auto finding = [&](const char* code, const std::string& evidence){
                Value d(rapidjson::kObjectType);
                d.AddMember("code", Value(code, a), a);
                d.AddMember("evidence", str(evidence, a), a);
                diagnostics.PushBack(d, a);
            };
            if(disabled) finding("audioDisabled", "DisableAudio=true");
            if(!manager.setupError.empty()) finding("setupFailed", manager.setupError);
            if(!available) finding("backendUnavailable", snap.reason);
            else if(snap.state != "playing") finding("sourceNotPlaying", snap.state);
            if(!buffer) finding("missingBuffer", "source has no buffer");
            else{
                const auto& info = buffer->getDebugInfo();
                if(!buffer->isReady()) finding("bufferNotReady", info.path);
                if(!info.loadError.empty()) finding("loadFailed", info.lastAttemptPath + ": " + info.loadError);
                if(!info.peak.empty() && std::all_of(info.peak.begin(), info.peak.end(), [](double p){ return p == 0; })){
                    finding("silentAsset", "all decoded samples are zero");
                }
            }
            if(manager.available && manager.gain == 0) finding("zeroMasterGain", "masterGain=0");
            if(available && snap.gain == 0) finding("zeroSourceGain", "gain=0");
            out.AddMember("diagnostics", diagnostics, a);
            return out;
        }
    }

    bool AudioInspector::parseUnsigned(const std::string& text, uint64& value){
        if(text.empty()) return false;
        uint64 parsed = 0;
        for(char c : text){
            if(c < '0' || c > '9') return false;
            const unsigned int digit = c - '0';
            if(parsed > (std::numeric_limits<uint64>::max() - digit) / 10) return false;
            parsed = parsed * 10 + digit;
        }
        value = parsed;
        return true;
    }

    void AudioInspector::write(rapidjson::Document& doc, int& status, AudioManager& manager,
                               const std::string& kind, uint64 id, const AudioQuery& query, bool disabled){
        doc.SetObject();
        auto& a = doc.GetAllocator();
        auto& state = manager.debugState();
        const auto snapshot = manager.debugSnapshot();
        doc.AddMember("frame", DebugJsonUtil::uint64Value(state.frame), a);
        doc.AddMember("timeSeconds", state.timeSeconds(), a);
        auto missing = [&]{
            status = 404;
            doc.AddMember("error", "unknown or expired audio id", a);
        };
        if(kind == "source"){
            auto it = state.sources.find(id);
            if(it == state.sources.end()){ missing(); return; }
            Value value = sourceValue(*it->second, it->second->debugSnapshot(), snapshot, state, disabled, true, a);
            for(auto member = value.MemberBegin(); member != value.MemberEnd(); ++member) doc.AddMember(member->name, member->value, a);
            return;
        }
        if(kind == "buffer"){
            auto it = state.buffers.find(id);
            if(it == state.buffers.end()){ missing(); return; }
            Value value = bufferValue(*it->second, state, true, a);
            for(auto member = value.MemberBegin(); member != value.MemberEnd(); ++member) doc.AddMember(member->name, member->value, a);
            return;
        }
        if(kind == "events"){
            const auto page = state.events(query.after, query.hasAfter, query.source, query.max);
            Value events(rapidjson::kArrayType);
            for(const auto& event : page.events){
                Value e(rapidjson::kObjectType);
                e.AddMember("sequence", DebugJsonUtil::uint64Value(event.sequence), a);
                e.AddMember("frame", DebugJsonUtil::uint64Value(event.frame), a);
                e.AddMember("timeSeconds", event.timeSeconds, a);
                e.AddMember("type", str(event.type, a), a);
                e.AddMember("sourceId", event.sourceId ? DebugJsonUtil::uint64Value(event.sourceId) : Value(), a);
                e.AddMember("bufferId", event.bufferId ? DebugJsonUtil::uint64Value(event.bufferId) : Value(), a);
                e.AddMember("path", str(event.path, a), a);
                e.AddMember("detail", str(event.detail, a), a);
                events.PushBack(e, a);
            }
            doc.AddMember("events", events, a);
            doc.AddMember("nextAfter", DebugJsonUtil::uint64Value(page.nextAfter), a);
            doc.AddMember("latestSequence", DebugJsonUtil::uint64Value(page.latestSequence), a);
            doc.AddMember("historyGap", page.historyGap, a);
            doc.AddMember("truncated", page.truncated, a);
            return;
        }
        if(kind == "sources" || kind == "buffers"){
            Value rows(rapidjson::kArrayType);
            uint64 total = 0, next = query.after;
            bool truncated = false;
            auto include = [&](uint64 rowId){
                ++total;
                if(rowId <= query.after) return false;
                if(rows.Size() == query.max){ truncated = true; return false; }
                next = rowId;
                return true;
            };
            if(kind == "sources"){
                for(const auto& entry : state.sources){
                    auto buffer = entry.second->getAudioBuffer();
                    const std::string path = buffer ? buffer->getDebugInfo().path : "";
                    if(path.find(query.path) == std::string::npos) continue;
                    if(!query.state.empty() && entry.second->debugPlaybackState() != query.state) continue;
                    if(include(entry.first)){
                        rows.PushBack(sourceValue(*entry.second, entry.second->debugSnapshot(), snapshot, state, disabled, false, a), a);
                    }
                }
            }else{
                for(const auto& entry : state.buffers){
                    if(entry.second->getDebugInfo().path.find(query.path) == std::string::npos) continue;
                    if(include(entry.first)) rows.PushBack(bufferValue(*entry.second, state, false, a), a);
                }
            }
            doc.AddMember(rapidjson::StringRef(kind == "sources" ? "sources" : "buffers"), rows, a);
            doc.AddMember("total", DebugJsonUtil::uint64Value(total), a);
            doc.AddMember("nextAfter", DebugJsonUtil::uint64Value(next), a);
            doc.AddMember("truncated", truncated, a);
            return;
        }
        doc.AddMember("backend", str(snapshot.backend, a), a);
        doc.AddMember("enabled", !disabled, a);
        doc.AddMember("setup", manager.isSetup(), a);
        doc.AddMember("available", snapshot.available, a);
        doc.AddMember("unavailableReason", snapshot.available ? Value() : str(disabled ? "audio disabled" : "audio backend unavailable", a), a);
        doc.AddMember("device", snapshot.device.empty() ? Value() : str(snapshot.device, a), a);
        doc.AddMember("setupError", snapshot.setupError.empty() ? Value() : str(snapshot.setupError, a), a);
        doc.AddMember("masterGain", number(snapshot.gain, snapshot.available), a);
        doc.AddMember("distanceModel", snapshot.available ? Value(distanceModel(snapshot.distanceModel), a) : Value(), a);
        Value listener(rapidjson::kObjectType);
        listener.AddMember("position", vec(snapshot.position, snapshot.available, a), a);
        listener.AddMember("velocity", vec(snapshot.velocity, snapshot.available, a), a);
        listener.AddMember("forward", vec(snapshot.forward, snapshot.available, a), a);
        listener.AddMember("up", vec(snapshot.up, snapshot.available, a), a);
        doc.AddMember("listener", listener, a);
        Value counts(rapidjson::kObjectType);
        std::map<std::string, unsigned int> states{{"initial", 0}, {"playing", 0}, {"paused", 0}, {"stopped", 0}, {"unavailable", 0}};
        for(const auto& entry : state.sources) ++states[entry.second->debugPlaybackState()];
        for(const auto& entry : states) counts.AddMember(str(entry.first, a), Value(entry.second), a);
        doc.AddMember("sourcesByState", counts, a);
        doc.AddMember("sourceCount", manager.getNumAudioSources(), a);
        doc.AddMember("bufferCount", manager.getNumAudioBuffers(), a);
        uint64 bytes = 0;
        for(const auto& entry : state.buffers) bytes += entry.second->getDebugInfo().bytes;
        doc.AddMember("decodedBytes", DebugJsonUtil::uint64Value(bytes), a);
        doc.AddMember("latestSequence", DebugJsonUtil::uint64Value(state.latestSequence()), a);
        doc.AddMember("outputMeasured", false, a);
    }
}
#endif
