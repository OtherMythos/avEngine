#ifdef DEBUG_SERVER

#include "RenderInspector.h"

#include "DebugJsonUtil.h"
#include "System/DebugServer/Render/ImageOps.h"

#include <algorithm>

namespace AV{
    //Out-of-line definitions: std::min/max odr-use these by reference.
    const int RenderInspector::MAX_CELLS_X;
    const int RenderInspector::MAX_CELLS_Y;
    const int RenderInspector::MAX_PNG_DIM;
    const int RenderInspector::ANALYSIS_W;
    const int RenderInspector::ANALYSIS_H;
    const int RenderInspector::MAX_CHANGED_CELLS;

    static void writeError(rapidjson::Document& doc, const std::string& message){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.AddMember("error", rapidjson::Value(message.c_str(), allocator), allocator);
    }

    static rapidjson::Value rgbArray(const uint8_t* rgb, rapidjson::Document::AllocatorType& allocator){
        rapidjson::Value arr(rapidjson::kArrayType);
        arr.PushBack(rgb[0], allocator);
        arr.PushBack(rgb[1], allocator);
        arr.PushBack(rgb[2], allocator);
        return arr;
    }

    static rapidjson::Value stringRows(const std::vector<std::string>& rows, rapidjson::Document::AllocatorType& allocator){
        rapidjson::Value arr(rapidjson::kArrayType);
        for(const std::string& row : rows){
            arr.PushBack(rapidjson::Value(row.c_str(), allocator), allocator);
        }
        return arr;
    }

    bool RenderInspector::validForm(const std::string& form){
        return form == "stats" || form == "grid" || form == "ascii" || form == "png";
    }

    void RenderInspector::writeFrame(rapidjson::Document& doc, int& status, const FrameParams& params, const CapturedFrame& captured){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();

        if(!validForm(params.form)){
            status = 400;
            writeError(doc, "unknown form '" + params.form + "'; expected stats, grid, ascii or png");
            return;
        }

        CapturedFrame frame = captured;

        doc.AddMember("frame", frame.frameNumber, allocator);
        doc.AddMember("captureWidth", frame.width, allocator);
        doc.AddMember("captureHeight", frame.height, allocator);

        if(params.hasRegion){
            frame = ImageOps::crop(frame, params.regionX, params.regionY, params.regionW, params.regionH);
            if(!frame.valid()){
                status = 400;
                writeError(doc, "region degenerates to zero pixels");
                return;
            }
            rapidjson::Value region(rapidjson::kArrayType);
            region.PushBack(params.regionX, allocator);
            region.PushBack(params.regionY, allocator);
            region.PushBack(params.regionW, allocator);
            region.PushBack(params.regionH, allocator);
            doc.AddMember("region", region, allocator);
        }

        if(params.form == "stats"){
            //Stats are computed on a bounded downsample: statistically identical for
            //summary purposes, and it caps the per-request cost.
            const CapturedFrame small = ImageOps::boxDownsample(frame, 320, 180);
            const ImageOps::FrameStats stats = ImageOps::computeStats(small);

            doc.AddMember("meanRgb", rgbArray(stats.meanRgb, allocator), allocator);

            rapidjson::Value lum(rapidjson::kObjectType);
            lum.AddMember("mean", stats.lumMean, allocator);
            lum.AddMember("min", stats.lumMin, allocator);
            lum.AddMember("max", stats.lumMax, allocator);
            rapidjson::Value hist(rapidjson::kArrayType);
            for(int i = 0; i < 10; i++) hist.PushBack(stats.histogram[i], allocator);
            lum.AddMember("histogram", hist, allocator);
            doc.AddMember("luminance", lum, allocator);

            rapidjson::Value dominant(rapidjson::kArrayType);
            for(const ImageOps::DominantColour& dc : stats.dominantColours){
                rapidjson::Value entry(rapidjson::kObjectType);
                entry.AddMember("rgb", rgbArray(dc.rgb, allocator), allocator);
                entry.AddMember("pct", dc.pct, allocator);
                dominant.PushBack(entry, allocator);
            }
            doc.AddMember("dominantColours", dominant, allocator);
        }else if(params.form == "grid" || params.form == "ascii"){
            const int cellsX = std::max(1, std::min(params.w, MAX_CELLS_X));
            const int cellsY = std::max(1, std::min(params.h, MAX_CELLS_Y));
            const CapturedFrame cells = ImageOps::boxDownsample(frame,
                static_cast<uint32_t>(cellsX), static_cast<uint32_t>(cellsY));

            doc.AddMember("cellsX", cells.width, allocator);
            doc.AddMember("cellsY", cells.height, allocator);
            if(params.form == "grid"){
                doc.AddMember("rows", stringRows(ImageOps::hexRows(cells), allocator), allocator);
            }else{
                doc.AddMember("rows", stringRows(ImageOps::asciiArt(cells), allocator), allocator);
            }
        }else{ //png
            const int maxDim = std::max(16, std::min(params.maxDim, MAX_PNG_DIM));
            CapturedFrame scaled = frame;
            if(frame.width > static_cast<uint32_t>(maxDim) || frame.height > static_cast<uint32_t>(maxDim)){
                const float scale = static_cast<float>(maxDim) / std::max(frame.width, frame.height);
                const uint32_t outW = std::max(1u, static_cast<uint32_t>(frame.width * scale));
                const uint32_t outH = std::max(1u, static_cast<uint32_t>(frame.height * scale));
                scaled = ImageOps::boxDownsample(frame, outW, outH);
            }

            const std::vector<uint8_t> png = ImageOps::encodePng(scaled);
            if(png.empty()){
                status = 503;
                writeError(doc, "png encoding failed");
                return;
            }
            doc.AddMember("width", scaled.width, allocator);
            doc.AddMember("height", scaled.height, allocator);
            const std::string encoded = ImageOps::base64(png);
            doc.AddMember("png_base64", rapidjson::Value(encoded.c_str(), allocator), allocator);
        }
    }

    CapturedFrame RenderInspector::toAnalysisFrame(const CapturedFrame& captured){
        if(!captured.valid()) return CapturedFrame();
        if(captured.width <= static_cast<uint32_t>(ANALYSIS_W) &&
           captured.height <= static_cast<uint32_t>(ANALYSIS_H)){
            return captured;
        }
        //Preserve aspect ratio within the analysis box.
        const float scale = std::min(static_cast<float>(ANALYSIS_W) / captured.width,
                                     static_cast<float>(ANALYSIS_H) / captured.height);
        const uint32_t outW = std::max(1u, static_cast<uint32_t>(captured.width * scale));
        const uint32_t outH = std::max(1u, static_cast<uint32_t>(captured.height * scale));
        CapturedFrame out = ImageOps::boxDownsample(captured, outW, outH);
        out.frameNumber = captured.frameNumber;
        return out;
    }

    void RenderInspector::writeHash(rapidjson::Document& doc, const CapturedFrame& analysis){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();
        doc.AddMember("frame", analysis.frameNumber, allocator);
        const std::string hex = ImageOps::hashToHex(ImageOps::dHash(analysis));
        doc.AddMember("dhash", rapidjson::Value(hex.c_str(), allocator), allocator);
    }

    void RenderInspector::writeSnapshot(rapidjson::Document& doc, const std::string& name, const CapturedFrame& analysis){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();
        doc.AddMember("name", rapidjson::Value(name.c_str(), allocator), allocator);
        doc.AddMember("frame", analysis.frameNumber, allocator);
        doc.AddMember("width", analysis.width, allocator);
        doc.AddMember("height", analysis.height, allocator);
        const std::string hex = ImageOps::hashToHex(ImageOps::dHash(analysis));
        doc.AddMember("dhash", rapidjson::Value(hex.c_str(), allocator), allocator);
    }

    void RenderInspector::writeCompare(rapidjson::Document& doc, const CapturedFrame& a, const CapturedFrame& b,
                                       int gridW, int gridH, float threshold){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();

        const uint64_t hashA = ImageOps::dHash(a);
        const uint64_t hashB = ImageOps::dHash(b);
        doc.AddMember("hammingDistance", ImageOps::hammingDistance(hashA, hashB), allocator);
        doc.AddMember("dhashA", rapidjson::Value(ImageOps::hashToHex(hashA).c_str(), allocator), allocator);
        doc.AddMember("dhashB", rapidjson::Value(ImageOps::hashToHex(hashB).c_str(), allocator), allocator);

        const ImageOps::DiffResult result = ImageOps::diff(a, b,
            static_cast<uint32_t>(gridW), static_cast<uint32_t>(gridH), threshold);
        if(!result.valid){
            doc.AddMember("error", "frames could not be compared", allocator);
            return;
        }

        doc.AddMember("gridW", result.gridW, allocator);
        doc.AddMember("gridH", result.gridH, allocator);
        doc.AddMember("changedFraction", result.changedFraction, allocator);
        doc.AddMember("meanDelta", result.meanDelta, allocator);

        rapidjson::Value cells(rapidjson::kArrayType);
        const size_t numCells = std::min<size_t>(MAX_CHANGED_CELLS, result.changedCells.size());
        for(size_t i = 0; i < numCells; i++){
            const ImageOps::DiffCell& cell = result.changedCells[i];
            rapidjson::Value entry(rapidjson::kObjectType);
            rapidjson::Value coord(rapidjson::kArrayType);
            coord.PushBack(cell.x, allocator); coord.PushBack(cell.y, allocator);
            entry.AddMember("cell", coord, allocator);
            entry.AddMember("delta", cell.delta, allocator);
            cells.PushBack(entry, allocator);
        }
        doc.AddMember("changedCells", cells, allocator);
        doc.AddMember("totalChangedCells", static_cast<uint64_t>(result.changedCells.size()), allocator);

        if(result.hasRegion){
            rapidjson::Value region(rapidjson::kObjectType);
            region.AddMember("x", result.regionX, allocator);
            region.AddMember("y", result.regionY, allocator);
            region.AddMember("w", result.regionW, allocator);
            region.AddMember("h", result.regionH, allocator);
            doc.AddMember("changedRegion", region, allocator);
        }
    }

    void RenderInspector::writeFind(rapidjson::Document& doc, const CapturedFrame& analysis,
                                    uint8_t r, uint8_t g, uint8_t b, int tolerance){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();
        doc.AddMember("frame", analysis.frameNumber, allocator);

        rapidjson::Value target(rapidjson::kArrayType);
        target.PushBack(r, allocator); target.PushBack(g, allocator); target.PushBack(b, allocator);
        doc.AddMember("rgb", target, allocator);
        doc.AddMember("tolerance", tolerance, allocator);

        //A region must cover at least ~0.02% of the frame to count, filtering stray pixels.
        const size_t minPixels = std::max<size_t>(4,
            (static_cast<size_t>(analysis.width) * analysis.height) / 5000);
        const std::vector<ImageOps::ColourMatch> matches =
            ImageOps::findColour(analysis, r, g, b, tolerance, minPixels, 10);

        rapidjson::Value arr(rapidjson::kArrayType);
        for(const ImageOps::ColourMatch& match : matches){
            rapidjson::Value entry(rapidjson::kObjectType);
            rapidjson::Value centre(rapidjson::kArrayType);
            centre.PushBack(match.centreX, allocator); centre.PushBack(match.centreY, allocator);
            entry.AddMember("centre", centre, allocator);
            rapidjson::Value bbox(rapidjson::kArrayType);
            bbox.PushBack(match.bboxX, allocator); bbox.PushBack(match.bboxY, allocator);
            bbox.PushBack(match.bboxW, allocator); bbox.PushBack(match.bboxH, allocator);
            entry.AddMember("bbox", bbox, allocator);
            entry.AddMember("pct", match.pct, allocator);
            arr.PushBack(entry, allocator);
        }
        doc.AddMember("matches", arr, allocator);
    }
}

#endif
