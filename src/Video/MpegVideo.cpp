#define PL_MPEG_IMPLEMENTATION
#include "MpegVideo.h"

#include <SDL_rwops.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

#if defined(__aarch64__) && defined(__ARM_NEON)
#include <arm_neon.h>
#endif

namespace AV{
    namespace{
        constexpr int kLumaScale = 298;
        constexpr int kRedCrScale = 409;
        constexpr int kGreenCbScale = -100;
        constexpr int kGreenCrScale = -208;
        constexpr int kBlueCbScale = 516;

#if defined(__aarch64__) && defined(__ARM_NEON)
        struct ColourTerms{
            int32x4_t low;
            int32x4_t high;
        };

        ColourTerms scaleSamples(uint8x8_t values, std::uint8_t zero, std::int16_t coefficient){
            const auto samples = vreinterpretq_s16_u16(vsubl_u8(values, vdup_n_u8(zero)));
            return {vmull_n_s16(vget_low_s16(samples), coefficient), vmull_high_n_s16(samples, coefficient)};
        }

        uint8x8_t convertChannel(ColourTerms brightness, ColourTerms chroma){
            const auto low = vqrshrun_n_s32(vaddq_s32(brightness.low, chroma.low), 8);
            const auto high = vqrshrun_n_s32(vaddq_s32(brightness.high, chroma.high), 8);
            return vqmovn_u16(vcombine_u16(low, high));
        }

        uint8x16_t convertPairs(ColourTerms even, ColourTerms odd, ColourTerms chroma){
            const auto interleaved = vzip_u8(convertChannel(even, chroma), convertChannel(odd, chroma));
            return vcombine_u8(interleaved.val[0], interleaved.val[1]);
        }

        void writeRgbaBlock(const std::uint8_t* luma, const std::uint8_t* cb, const std::uint8_t* cr,
                            std::uint8_t* dest){
            //Even and odd pixels share each chroma sample across a horizontal pair.
            const auto y = vld2_u8(luma);
            const auto u = vld1_u8(cb);
            const auto v = vld1_u8(cr);
            const auto even = scaleSamples(y.val[0], 16, kLumaScale);
            const auto odd = scaleSamples(y.val[1], 16, kLumaScale);
            const auto red = scaleSamples(v, 128, kRedCrScale);
            const auto greenCb = scaleSamples(u, 128, kGreenCbScale);
            const auto greenCr = scaleSamples(v, 128, kGreenCrScale);
            const ColourTerms green{vaddq_s32(greenCb.low, greenCr.low), vaddq_s32(greenCb.high, greenCr.high)};
            const auto blue = scaleSamples(u, 128, kBlueCbScale);
            const uint8x16x4_t rgba{{
                convertPairs(even, odd, red),
                convertPairs(even, odd, green),
                convertPairs(even, odd, blue),
                vdupq_n_u8(255)
            }};
            vst4q_u8(dest, rgba);
        }
#endif

        void loadVideoBytes(plm_buffer_t* buffer, void* user){
            //Mirror pl_mpeg's stdio loader so the same path can stream from SDL's APK files.
            plm_buffer_discard_read_bytes(buffer);
            const auto count = SDL_RWread(static_cast<SDL_RWops*>(user), buffer->bytes + buffer->length,
                                          1, buffer->capacity - buffer->length);
            buffer->length += count;
            buffer->has_ended = count == 0;
        }

        void seekVideoBytes(plm_buffer_t*, std::size_t offset, void* user){
            SDL_RWseek(static_cast<SDL_RWops*>(user), static_cast<Sint64>(offset), RW_SEEK_SET);
        }

        std::size_t tellVideoBytes(plm_buffer_t*, void* user){
            return static_cast<std::size_t>(SDL_RWtell(static_cast<SDL_RWops*>(user)));
        }
    }

    void MpegVideo::CloseFile::operator()(SDL_RWops* file) const{
        SDL_RWclose(file);
    }

    void MpegVideo::DestroyDecoder::operator()(plm_t* decoder) const{
        plm_destroy(decoder);
    }

    MpegVideo::MpegVideo(const std::string& path)
        : mFile(SDL_RWFromFile(path.c_str(), "rb")){
        if(!mFile){
            throw std::runtime_error("Cannot open MPEG-1 video '" + path + "'.");
        }

        const auto size = SDL_RWsize(mFile.get());
        if(size <= 0 || static_cast<std::uint64_t>(size) > std::numeric_limits<std::size_t>::max()){
            throw std::runtime_error("Cannot read MPEG-1 video '" + path + "'.");
        }

        auto* buffer = plm_buffer_create_with_callbacks(loadVideoBytes, seekVideoBytes, tellVideoBytes,
                                                       static_cast<std::size_t>(size), mFile.get());
        mDecoder.reset(plm_create_with_buffer(buffer, true));
        plm_set_audio_enabled(mDecoder.get(), false);

        const double frameRate = getFrameRate();
        if(frameRate <= 0.0 || !decodeFrame()){
            throw std::runtime_error("No decodable MPEG-1 video in '" + path + "'.");
        }
        mFrameDuration = 1.0 / frameRate;
    }

    unsigned MpegVideo::getWidth() const{
        return plm_get_width(mDecoder.get());
    }

    unsigned MpegVideo::getHeight() const{
        return plm_get_height(mDecoder.get());
    }

    double MpegVideo::getFrameRate() const{
        return plm_get_framerate(mDecoder.get());
    }

    double MpegVideo::getPixelAspectRatio() const{
        //The MPEG-1 table gives pixel height divided by width.
        return 1.0 / plm_get_pixel_aspect_ratio(mDecoder.get());
    }

    bool MpegVideo::decodeFrame(){
        mFrame = plm_decode_video(mDecoder.get());
        mEnded = mFrame == nullptr;
        return !mEnded;
    }

    bool MpegVideo::advance(double seconds){
        if(!std::isfinite(seconds) || seconds < 0.0){
            throw std::invalid_argument("Video delta must be finite and non-negative.");
        }
        if(mEnded || seconds == 0.0) return false;

        //Limit catch-up to one frame per update and at most one frame of remaining debt.
        mElapsed = std::min(mElapsed + seconds, 2.0 * mFrameDuration);
        if(mElapsed < mFrameDuration) return false;

        mElapsed -= mFrameDuration;
        return decodeFrame();
    }

    void MpegVideo::rewind(){
        plm_rewind(mDecoder.get());
        mElapsed = 0.0;
        if(!decodeFrame()){
            throw std::runtime_error("Cannot decode first MPEG-1 frame after rewind.");
        }
    }

    void MpegVideo::writeRgba(std::uint8_t* dest, std::size_t bytesPerRow) const noexcept{
        if(!mFrame) return;

        const auto channel = [](int value){
            return static_cast<std::uint8_t>(std::clamp((value + 128) >> 8, 0, 255));
        };

        //BT.601 limited-range Y'CbCr. The decoded planes can have padding after each row.
        const auto& frame = *mFrame;
        for(unsigned y = 0; y < frame.height; ++y){
            const auto* luma = frame.y.data + y * frame.y.width;
            const auto* cb = frame.cb.data + (y / 2) * frame.cb.width;
            const auto* cr = frame.cr.data + (y / 2) * frame.cr.width;
            auto* row = dest + y * bytesPerRow;
            unsigned x = 0;
#if defined(__aarch64__) && defined(__ARM_NEON)
            for(; x + 16 <= frame.width; x += 16){
                writeRgbaBlock(luma + x, cb + x / 2, cr + x / 2, row + x * 4);
            }
#endif
            for(; x < frame.width; x += 2){
                const int u = cb[x / 2] - 128;
                const int v = cr[x / 2] - 128;
                const int r = kRedCrScale * v;
                const int g = kGreenCbScale * u + kGreenCrScale * v;
                const int b = kBlueCbScale * u;
                const auto writePixel = [&](unsigned column){
                    const int brightness = kLumaScale * (luma[column] - 16);
                    row[4 * column] = channel(brightness + r);
                    row[4 * column + 1] = channel(brightness + g);
                    row[4 * column + 2] = channel(brightness + b);
                    row[4 * column + 3] = 255;
                };
                writePixel(x);
                if(x + 1 < frame.width) writePixel(x + 1);
            }
        }
    }
}
