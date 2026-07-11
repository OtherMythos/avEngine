#ifdef DEBUG_SERVER

#include "ImageOps.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include "stb/stb_image_write.h"

#include <algorithm>
#include <map>
#include <cmath>

namespace AV{
    namespace ImageOps{
        CapturedFrame crop(const CapturedFrame& frame, float x, float y, float w, float h){
            CapturedFrame out;
            if(!frame.valid()) return out;

            x = std::max(0.0f, std::min(1.0f, x));
            y = std::max(0.0f, std::min(1.0f, y));
            w = std::max(0.0f, std::min(1.0f - x, w));
            h = std::max(0.0f, std::min(1.0f - y, h));

            const uint32_t px = static_cast<uint32_t>(x * frame.width);
            const uint32_t py = static_cast<uint32_t>(y * frame.height);
            uint32_t pw = static_cast<uint32_t>(std::round(w * frame.width));
            uint32_t ph = static_cast<uint32_t>(std::round(h * frame.height));
            if(px + pw > frame.width) pw = frame.width - px;
            if(py + ph > frame.height) ph = frame.height - py;
            if(pw == 0 || ph == 0) return out;

            out.width = pw;
            out.height = ph;
            out.rgb.resize(static_cast<size_t>(pw) * ph * 3);
            for(uint32_t row = 0; row < ph; row++){
                const uint8_t* src = &frame.rgb[(static_cast<size_t>(py + row) * frame.width + px) * 3];
                uint8_t* dst = &out.rgb[static_cast<size_t>(row) * pw * 3];
                std::copy(src, src + static_cast<size_t>(pw) * 3, dst);
            }
            return out;
        }

        CapturedFrame boxDownsample(const CapturedFrame& frame, uint32_t outW, uint32_t outH){
            CapturedFrame out;
            if(!frame.valid() || outW == 0 || outH == 0) return out;

            outW = std::min(outW, frame.width);
            outH = std::min(outH, frame.height);

            out.width = outW;
            out.height = outH;
            out.rgb.resize(static_cast<size_t>(outW) * outH * 3);

            for(uint32_t oy = 0; oy < outH; oy++){
                //Source row range for this output row. Integer boundaries guarantee every
                //source pixel lands in exactly one cell.
                const uint32_t y0 = static_cast<uint32_t>((static_cast<uint64_t>(oy) * frame.height) / outH);
                const uint32_t y1 = static_cast<uint32_t>((static_cast<uint64_t>(oy + 1) * frame.height) / outH);
                for(uint32_t ox = 0; ox < outW; ox++){
                    const uint32_t x0 = static_cast<uint32_t>((static_cast<uint64_t>(ox) * frame.width) / outW);
                    const uint32_t x1 = static_cast<uint32_t>((static_cast<uint64_t>(ox + 1) * frame.width) / outW);

                    uint64_t sum[3] = {0, 0, 0};
                    for(uint32_t sy = y0; sy < y1; sy++){
                        const uint8_t* src = &frame.rgb[(static_cast<size_t>(sy) * frame.width + x0) * 3];
                        for(uint32_t sx = x0; sx < x1; sx++){
                            sum[0] += src[0];
                            sum[1] += src[1];
                            sum[2] += src[2];
                            src += 3;
                        }
                    }
                    const uint64_t count = static_cast<uint64_t>(y1 - y0) * (x1 - x0);
                    uint8_t* dst = &out.rgb[(static_cast<size_t>(oy) * outW + ox) * 3];
                    dst[0] = static_cast<uint8_t>(sum[0] / count);
                    dst[1] = static_cast<uint8_t>(sum[1] / count);
                    dst[2] = static_cast<uint8_t>(sum[2] / count);
                }
            }
            return out;
        }

        float luminance(uint8_t r, uint8_t g, uint8_t b){
            return (0.2126f * r + 0.7152f * g + 0.0722f * b) / 255.0f;
        }

        FrameStats computeStats(const CapturedFrame& frame){
            FrameStats stats = {};
            if(!frame.valid()) return stats;

            const size_t numPixels = static_cast<size_t>(frame.width) * frame.height;

            uint64_t sum[3] = {0, 0, 0};
            double lumSum = 0.0;
            float lumMin = 1.0f, lumMax = 0.0f;
            uint64_t histogram[10] = {};

            //4 bits per channel quantisation for dominant colour binning: 4096 bins.
            struct Bin{ uint64_t count = 0; uint64_t sum[3] = {0, 0, 0}; };
            std::map<uint16_t, Bin> bins;

            const uint8_t* p = frame.rgb.data();
            for(size_t i = 0; i < numPixels; i++, p += 3){
                sum[0] += p[0];
                sum[1] += p[1];
                sum[2] += p[2];

                const float lum = luminance(p[0], p[1], p[2]);
                lumSum += lum;
                lumMin = std::min(lumMin, lum);
                lumMax = std::max(lumMax, lum);
                int bucket = static_cast<int>(lum * 10.0f);
                if(bucket > 9) bucket = 9;
                histogram[bucket]++;

                const uint16_t key = static_cast<uint16_t>(((p[0] >> 4) << 8) | ((p[1] >> 4) << 4) | (p[2] >> 4));
                Bin& bin = bins[key];
                bin.count++;
                bin.sum[0] += p[0];
                bin.sum[1] += p[1];
                bin.sum[2] += p[2];
            }

            for(int c = 0; c < 3; c++) stats.meanRgb[c] = static_cast<uint8_t>(sum[c] / numPixels);
            stats.lumMean = static_cast<float>(lumSum / numPixels);
            stats.lumMin = lumMin;
            stats.lumMax = lumMax;
            for(int i = 0; i < 10; i++) stats.histogram[i] = 100.0f * histogram[i] / numPixels;

            //Top 5 bins by population, reported as the mean colour of each bin.
            typedef std::pair<const uint16_t, Bin> BinEntry;
            std::vector<const BinEntry*> sorted;
            sorted.reserve(bins.size());
            for(const BinEntry& entry : bins) sorted.push_back(&entry);
            std::sort(sorted.begin(), sorted.end(), [](const BinEntry* a, const BinEntry* b){
                return a->second.count > b->second.count;
            });
            const size_t numDominant = std::min<size_t>(5, sorted.size());
            for(size_t i = 0; i < numDominant; i++){
                const Bin& bin = sorted[i]->second;
                DominantColour dc;
                for(int c = 0; c < 3; c++) dc.rgb[c] = static_cast<uint8_t>(bin.sum[c] / bin.count);
                dc.pct = 100.0f * bin.count / numPixels;
                stats.dominantColours.push_back(dc);
            }

            return stats;
        }

        std::vector<std::string> asciiArt(const CapturedFrame& frame){
            static const char ramp[] = " .:-=+*#%@";
            std::vector<std::string> rows;
            if(!frame.valid()) return rows;

            rows.reserve(frame.height);
            for(uint32_t y = 0; y < frame.height; y++){
                std::string row;
                row.reserve(frame.width);
                const uint8_t* p = &frame.rgb[static_cast<size_t>(y) * frame.width * 3];
                for(uint32_t x = 0; x < frame.width; x++, p += 3){
                    int idx = static_cast<int>(luminance(p[0], p[1], p[2]) * 10.0f);
                    if(idx > 9) idx = 9;
                    row.push_back(ramp[idx]);
                }
                rows.push_back(std::move(row));
            }
            return rows;
        }

        std::vector<std::string> hexRows(const CapturedFrame& frame){
            static const char hex[] = "0123456789abcdef";
            std::vector<std::string> rows;
            if(!frame.valid()) return rows;

            rows.reserve(frame.height);
            for(uint32_t y = 0; y < frame.height; y++){
                std::string row;
                row.reserve(static_cast<size_t>(frame.width) * 6);
                const uint8_t* p = &frame.rgb[static_cast<size_t>(y) * frame.width * 3];
                for(uint32_t x = 0; x < frame.width; x++, p += 3){
                    for(int c = 0; c < 3; c++){
                        row.push_back(hex[p[c] >> 4]);
                        row.push_back(hex[p[c] & 0xF]);
                    }
                }
                rows.push_back(std::move(row));
            }
            return rows;
        }

        std::vector<uint8_t> encodePng(const CapturedFrame& frame){
            std::vector<uint8_t> out;
            if(!frame.valid()) return out;

            auto writeFunc = [](void* context, void* data, int size){
                std::vector<uint8_t>* buffer = static_cast<std::vector<uint8_t>*>(context);
                const uint8_t* bytes = static_cast<const uint8_t*>(data);
                buffer->insert(buffer->end(), bytes, bytes + size);
            };

            const int result = stbi_write_png_to_func(writeFunc, &out,
                static_cast<int>(frame.width), static_cast<int>(frame.height), 3,
                frame.rgb.data(), static_cast<int>(frame.width) * 3);
            if(result == 0) out.clear();
            return out;
        }

        std::string base64(const std::vector<uint8_t>& data){
            static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            std::string out;
            out.reserve(((data.size() + 2) / 3) * 4);

            size_t i = 0;
            while(i + 2 < data.size()){
                const uint32_t n = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
                out.push_back(chars[(n >> 18) & 63]);
                out.push_back(chars[(n >> 12) & 63]);
                out.push_back(chars[(n >> 6) & 63]);
                out.push_back(chars[n & 63]);
                i += 3;
            }
            const size_t remaining = data.size() - i;
            if(remaining == 1){
                const uint32_t n = data[i] << 16;
                out.push_back(chars[(n >> 18) & 63]);
                out.push_back(chars[(n >> 12) & 63]);
                out.push_back('=');
                out.push_back('=');
            }else if(remaining == 2){
                const uint32_t n = (data[i] << 16) | (data[i + 1] << 8);
                out.push_back(chars[(n >> 18) & 63]);
                out.push_back(chars[(n >> 12) & 63]);
                out.push_back(chars[(n >> 6) & 63]);
                out.push_back('=');
            }
            return out;
        }
    }
}

#endif
