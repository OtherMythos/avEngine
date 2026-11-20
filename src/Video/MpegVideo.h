#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

#include "ThirdParty/pl_mpeg/pl_mpeg.h"

struct SDL_RWops;

namespace AV{
    class MpegVideo{
    public:
        explicit MpegVideo(const std::string& path);

        unsigned getWidth() const;
        unsigned getHeight() const;
        double getFrameRate() const;
        double getPixelAspectRatio() const;
        bool hasEnded() const { return mEnded; }

        bool advance(double seconds);
        void rewind();
        void writeRgba(std::uint8_t* dest, std::size_t bytesPerRow) const noexcept;

    private:
        struct CloseFile{
            void operator()(SDL_RWops* file) const;
        };
        struct DestroyDecoder{
            void operator()(plm_t* decoder) const;
        };

        bool decodeFrame();

        std::unique_ptr<SDL_RWops, CloseFile> mFile;
        std::unique_ptr<plm_t, DestroyDecoder> mDecoder;
        plm_frame_t* mFrame = nullptr;
        double mFrameDuration = 0.0;
        double mElapsed = 0.0;
        bool mEnded = false;
    };
}
