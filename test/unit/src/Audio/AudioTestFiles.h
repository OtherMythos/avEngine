#pragma once

#include "sndfile.h"
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace AV{
    class AudioTestFile{
    public:
        std::string path;
        std::vector<short> samples;
        AudioTestFile(size_t frames = 4800, int channels = 2, int rate = 48000){
            static std::atomic<unsigned> next{0};
            path = (std::filesystem::temp_directory_path() / ("av-audio-" +
                std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "-" +
                std::to_string(next++) + ".wav")).string();
            samples.resize(frames * channels);
            for(size_t i = 0; i < samples.size(); ++i) samples[i] = static_cast<short>((i % 127) * 100 - 6350);
            SF_INFO info = {};
            info.samplerate = rate;
            info.channels = channels;
            info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
            auto* file = sf_open(path.c_str(), SFM_WRITE, &info);
            if(!file) throw std::runtime_error("Could not create test WAV");
            sf_writef_short(file, samples.data(), frames);
            sf_close(file);
        }
        ~AudioTestFile(){ std::filesystem::remove(path); }
    };
}
