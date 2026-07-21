#ifdef DEBUG_SERVER

#include "ImageOps.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include "stb/stb_image_write.h"

#include <algorithm>
#include <map>
#include <cmath>
#include <cstdlib>

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

        uint64_t dHash(const CapturedFrame& frame){
            if(!frame.valid()) return 0;

            //9 wide so each of the 8 comparisons per row has a right-hand neighbour.
            const CapturedFrame small = boxDownsample(frame, 9, 8);
            if(small.width < 2 || small.height < 1) return 0;

            uint64_t hash = 0;
            int bit = 0;
            for(uint32_t y = 0; y < small.height && bit < 64; y++){
                for(uint32_t x = 0; x + 1 < small.width && bit < 64; x++){
                    const uint8_t* a = &small.rgb[(static_cast<size_t>(y) * small.width + x) * 3];
                    const uint8_t* b = a + 3;
                    if(luminance(a[0], a[1], a[2]) > luminance(b[0], b[1], b[2])){
                        hash |= (uint64_t(1) << bit);
                    }
                    bit++;
                }
            }
            return hash;
        }

        int hammingDistance(uint64_t a, uint64_t b){
            uint64_t diffBits = a ^ b;
            int count = 0;
            while(diffBits){
                count += static_cast<int>(diffBits & 1);
                diffBits >>= 1;
            }
            return count;
        }

        std::string hashToHex(uint64_t hash){
            static const char hex[] = "0123456789abcdef";
            std::string out;
            out.reserve(16);
            for(int i = 15; i >= 0; i--){
                out.push_back(hex[(hash >> (i * 4)) & 0xF]);
            }
            return out;
        }

        DiffResult diff(const CapturedFrame& a, const CapturedFrame& b,
                        uint32_t gridW, uint32_t gridH, float threshold){
            DiffResult result;
            if(!a.valid() || !b.valid() || gridW == 0 || gridH == 0) return result;

            //Reduce both to the same grid so frames of differing size still compare.
            const CapturedFrame ga = boxDownsample(a, gridW, gridH);
            const CapturedFrame gb = boxDownsample(b, gridW, gridH);
            if(!ga.valid() || !gb.valid()) return result;
            if(ga.width != gb.width || ga.height != gb.height) return result;

            result.valid = true;
            result.gridW = ga.width;
            result.gridH = ga.height;

            uint32_t minX = ga.width, minY = ga.height, maxX = 0, maxY = 0;
            size_t changed = 0;
            double deltaSum = 0.0;

            for(uint32_t y = 0; y < ga.height; y++){
                for(uint32_t x = 0; x < ga.width; x++){
                    const size_t i = (static_cast<size_t>(y) * ga.width + x) * 3;
                    const int dr = std::abs(static_cast<int>(ga.rgb[i]) - static_cast<int>(gb.rgb[i]));
                    const int dg = std::abs(static_cast<int>(ga.rgb[i + 1]) - static_cast<int>(gb.rgb[i + 1]));
                    const int db = std::abs(static_cast<int>(ga.rgb[i + 2]) - static_cast<int>(gb.rgb[i + 2]));
                    const float delta = static_cast<float>(dr + dg + db) / (3.0f * 255.0f);
                    deltaSum += delta;

                    if(delta > threshold){
                        changed++;
                        result.changedCells.push_back({x, y, delta});
                        if(x < minX) minX = x;
                        if(y < minY) minY = y;
                        if(x > maxX) maxX = x;
                        if(y > maxY) maxY = y;
                    }
                }
            }

            const size_t totalCells = static_cast<size_t>(ga.width) * ga.height;
            result.changedFraction = static_cast<float>(changed) / totalCells;
            result.meanDelta = static_cast<float>(deltaSum / totalCells);

            std::sort(result.changedCells.begin(), result.changedCells.end(),
                [](const DiffCell& l, const DiffCell& r){ return l.delta > r.delta; });

            if(changed > 0){
                result.hasRegion = true;
                result.regionX = static_cast<float>(minX) / ga.width;
                result.regionY = static_cast<float>(minY) / ga.height;
                //+1 so a single changed cell still has the width of that cell.
                result.regionW = static_cast<float>(maxX - minX + 1) / ga.width;
                result.regionH = static_cast<float>(maxY - minY + 1) / ga.height;
            }

            return result;
        }

        std::vector<ColourMatch> findColour(const CapturedFrame& frame,
                                            uint8_t r, uint8_t g, uint8_t b,
                                            int tolerance, size_t minPixels, size_t maxMatches){
            std::vector<ColourMatch> matches;
            if(!frame.valid()) return matches;

            const uint32_t w = frame.width, h = frame.height;
            const size_t numPixels = static_cast<size_t>(w) * h;

            //Mask of pixels within tolerance of the target colour.
            std::vector<uint8_t> mask(numPixels, 0);
            for(size_t i = 0; i < numPixels; i++){
                const uint8_t* p = &frame.rgb[i * 3];
                const int dr = std::abs(static_cast<int>(p[0]) - static_cast<int>(r));
                const int dg = std::abs(static_cast<int>(p[1]) - static_cast<int>(g));
                const int db = std::abs(static_cast<int>(p[2]) - static_cast<int>(b));
                if(dr <= tolerance && dg <= tolerance && db <= tolerance) mask[i] = 1;
            }

            //Flood fill (4-connected) each unvisited masked pixel into a region.
            struct Region{ uint32_t minX, minY, maxX, maxY; double sumX, sumY; size_t count; };
            std::vector<Region> regions;
            std::vector<uint32_t> stack;

            for(uint32_t y = 0; y < h; y++){
                for(uint32_t x = 0; x < w; x++){
                    const size_t start = static_cast<size_t>(y) * w + x;
                    if(!mask[start]) continue;

                    Region region{x, y, x, y, 0.0, 0.0, 0};
                    stack.clear();
                    stack.push_back(static_cast<uint32_t>(start));
                    mask[start] = 0; //Mark visited as we push.

                    while(!stack.empty()){
                        const uint32_t idx = stack.back();
                        stack.pop_back();
                        const uint32_t px = idx % w;
                        const uint32_t py = idx / w;

                        region.count++;
                        region.sumX += px;
                        region.sumY += py;
                        if(px < region.minX) region.minX = px;
                        if(py < region.minY) region.minY = py;
                        if(px > region.maxX) region.maxX = px;
                        if(py > region.maxY) region.maxY = py;

                        if(px > 0 && mask[idx - 1]){ mask[idx - 1] = 0; stack.push_back(idx - 1); }
                        if(px + 1 < w && mask[idx + 1]){ mask[idx + 1] = 0; stack.push_back(idx + 1); }
                        if(py > 0 && mask[idx - w]){ mask[idx - w] = 0; stack.push_back(idx - w); }
                        if(py + 1 < h && mask[idx + w]){ mask[idx + w] = 0; stack.push_back(idx + w); }
                    }

                    if(region.count >= minPixels) regions.push_back(region);
                }
            }

            std::sort(regions.begin(), regions.end(),
                [](const Region& l, const Region& r){ return l.count > r.count; });

            const size_t numMatches = std::min(maxMatches, regions.size());
            for(size_t i = 0; i < numMatches; i++){
                const Region& region = regions[i];
                ColourMatch match;
                match.centreX = static_cast<float>(region.sumX / region.count) / w;
                match.centreY = static_cast<float>(region.sumY / region.count) / h;
                match.bboxX = static_cast<float>(region.minX) / w;
                match.bboxY = static_cast<float>(region.minY) / h;
                match.bboxW = static_cast<float>(region.maxX - region.minX + 1) / w;
                match.bboxH = static_cast<float>(region.maxY - region.minY + 1) / h;
                match.pct = 100.0f * region.count / numPixels;
                matches.push_back(match);
            }

            return matches;
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
