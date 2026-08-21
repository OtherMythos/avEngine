#if defined(DEBUG_SERVER) || defined(FLIGHT_RECORDER)

#include "FrameCapture.h"

#include "System/BaseSingleton.h"
#include "Window/Window.h"
#include "Logger/Log.h"

#include "OgreRoot.h"
#include "OgreWindow.h"
#include "OgreTextureGpu.h"
#include "OgreImage2.h"
#include "OgreTextureBox.h"
#include "OgrePixelFormatGpuUtils.h"
#include "OgreException.h"

#include <chrono>
#include <vector>
#include <algorithm>

#include <algorithm>
#include <chrono>

namespace AV{

    //Split of the last readback, for the flight recorder's cost reporting.
    double FrameCapture::sLastDownloadUs = 0.0;
    double FrameCapture::sLastUnpackUs = 0.0;
    void FrameCapture::initialise(){
        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        if(!root) return;
        root->addFrameListener(this);
        mRegistered = true;
    }

    void FrameCapture::shutdown(){
        if(mShutdown.exchange(true)) return;

        if(mRegistered){
            Ogre::Root* root = Ogre::Root::getSingletonPtr();
            if(root) root->removeFrameListener(this);
            mRegistered = false;
        }

        //Wake blocked requesters; their predicate observes mShutdown and they fail out.
        {
            std::lock_guard<std::mutex> lock(mMutex);
        }
        mCv.notify_all();
    }

    bool FrameCapture::requestCapture(CapturedFrame& out, std::string& outError, uint32_t timeoutMs){
        if(mShutdown.load()){
            outError = "shutting down";
            return false;
        }

        std::unique_lock<std::mutex> lock(mMutex);
        mPending = true;
        mDone = false;

        const bool completed = mCv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]{
            return mDone || mShutdown.load();
        });

        if(!completed || mShutdown.load()){
            //Timed out (engine paused/backgrounded, so no frames are rendering) or torn down.
            mPending = false;
            outError = mShutdown.load() ? "shutting down" : "no frame was rendered within the timeout (engine paused or in background)";
            return false;
        }

        if(!mSuccess){
            outError = mError;
            return false;
        }

        //Copy rather than move: a second concurrent requester may be about to read the
        //same capture.
        out = mFrame;
        return true;
    }

    bool FrameCapture::frameRenderingQueued(const Ogre::FrameEvent& evt){
        //Main thread, render commands issued, final swap not yet performed: the window
        //drawable is still alive here.
        std::lock_guard<std::mutex> lock(mMutex);
        mFrameNumber++;

        if(!mPending) return true;

        mSuccess = _performCapture();
        mPending = false;
        mDone = true;
        mCv.notify_all();
        return true;
    }

    bool FrameCapture::_performCapture(){
        const bool result = readColourBuffer(mFrame, mError);
        if(result) mFrame.frameNumber = mFrameNumber;
        return result;
    }

    bool FrameCapture::readColourBuffer(CapturedFrame& out, std::string& outError){
        Window* window = BaseSingleton::getWindow();
        if(!window){
            outError = "window not available";
            return false;
        }
        Ogre::TextureGpu* texture = window->getRenderTexture();
        if(!texture){
            outError = "render texture not available";
            return false;
        }

        try{
            //Synchronous GPU->CPU readback. automaticResolve handles MSAA window buffers.
            const auto downloadStart = std::chrono::steady_clock::now();
            Ogre::Image2 image;
            image.convertFromTexture(texture, 0u, 0u, true);
            sLastDownloadUs = std::chrono::duration<double, std::micro>(
                std::chrono::steady_clock::now() - downloadStart).count();
            const auto unpackStart = std::chrono::steady_clock::now();

            const uint32_t width = image.getWidth();
            const uint32_t height = image.getHeight();
            if(width == 0 || height == 0){
                outError = "captured image is empty";
                return false;
            }

            out.width = width;
            out.height = height;
            out.rgb.resize(static_cast<size_t>(width) * height * 3);

            //The stored bytes of a UNORM_SRGB window buffer are display-referred, which is
            //what an agent asking "what is on screen" wants.
            Ogre::TextureBox box = image.getData(0);
            const Ogre::PixelFormatGpu format = image.getPixelFormat();
            uint8_t* dst = out.rgb.data();

            //Window buffers are almost always 8-bit RGBA or BGRA; walk those directly.
            //A capture at retina resolutions is millions of pixels, and the generic
            //per-pixel getColourAt unpack costs over a second at that size.
            const bool rgba8 = (format == Ogre::PFG_RGBA8_UNORM || format == Ogre::PFG_RGBA8_UNORM_SRGB);
            const bool bgra8 = (format == Ogre::PFG_BGRA8_UNORM || format == Ogre::PFG_BGRA8_UNORM_SRGB);
            if(rgba8 || bgra8){
                const uint32_t rIdx = bgra8 ? 2 : 0;
                const uint32_t bIdx = bgra8 ? 0 : 2;
                for(uint32_t y = 0; y < height; y++){
                    const uint8_t* src = reinterpret_cast<const uint8_t*>(box.at(0, y, 0));
                    for(uint32_t x = 0; x < width; x++){
                        dst[0] = src[rIdx];
                        dst[1] = src[1];
                        dst[2] = src[bIdx];
                        src += 4;
                        dst += 3;
                    }
                }
            }else{
                //Fallback for exotic formats: getColourAt handles the unpack generically.
                for(uint32_t y = 0; y < height; y++){
                    for(uint32_t x = 0; x < width; x++){
                        const Ogre::ColourValue colour = box.getColourAt(x, y, 0, format);
                        dst[0] = static_cast<uint8_t>(std::max(0.0f, std::min(1.0f, colour.r)) * 255.0f + 0.5f);
                        dst[1] = static_cast<uint8_t>(std::max(0.0f, std::min(1.0f, colour.g)) * 255.0f + 0.5f);
                        dst[2] = static_cast<uint8_t>(std::max(0.0f, std::min(1.0f, colour.b)) * 255.0f + 0.5f);
                        dst += 3;
                    }
                }
            }
            sLastUnpackUs = std::chrono::duration<double, std::micro>(
                std::chrono::steady_clock::now() - unpackStart).count();
        }catch(Ogre::Exception& e){
            AV_ERROR("FrameCapture readback failed: {}", e.getDescription());
            outError = "readback failed: " + e.getDescription();
            return false;
        }

        return true;
    }

    bool FrameCapture::readColourBufferDownsampled(uint32_t outWidth, uint32_t outHeight, bool fastSample,
                                                   CapturedFrame& out, std::string& outError){
        Window* window = BaseSingleton::getWindow();
        if(!window){
            outError = "window not available";
            return false;
        }
        Ogre::TextureGpu* texture = window->getRenderTexture();
        if(!texture){
            outError = "render texture not available";
            return false;
        }
        if(outWidth == 0 || outHeight == 0){
            outError = "invalid target size";
            return false;
        }

        try{
            const auto downloadStart = std::chrono::steady_clock::now();
            Ogre::Image2 image;
            image.convertFromTexture(texture, 0u, 0u, true);
            sLastDownloadUs = std::chrono::duration<double, std::micro>(
                std::chrono::steady_clock::now() - downloadStart).count();

            const uint32_t width = image.getWidth();
            const uint32_t height = image.getHeight();
            if(width == 0 || height == 0){
                outError = "captured image is empty";
                return false;
            }

            //Fit the source inside the requested box rather than stretching it to fill it: a
            //distorted frame actively misleads someone diagnosing a visual bug. Never
            //upsamples either.
            uint32_t dstWidth = outWidth;
            uint32_t dstHeight = outHeight;
            {
                const double scale = std::min(static_cast<double>(outWidth) / width,
                                              static_cast<double>(outHeight) / height);
                if(scale < 1.0){
                    dstWidth = std::max<uint32_t>(1u, static_cast<uint32_t>(width * scale + 0.5));
                    dstHeight = std::max<uint32_t>(1u, static_cast<uint32_t>(height * scale + 0.5));
                }else{
                    dstWidth = width;
                    dstHeight = height;
                }
            }

            const auto unpackStart = std::chrono::steady_clock::now();

            Ogre::TextureBox box = image.getData(0);
            const Ogre::PixelFormatGpu format = image.getPixelFormat();
            const bool rgba8 = (format == Ogre::PFG_RGBA8_UNORM || format == Ogre::PFG_RGBA8_UNORM_SRGB);
            const bool bgra8 = (format == Ogre::PFG_BGRA8_UNORM || format == Ogre::PFG_BGRA8_UNORM_SRGB);

            out.width = dstWidth;
            out.height = dstHeight;
            out.rgb.assign(static_cast<size_t>(dstWidth) * dstHeight * 3, 0);

            if(!rgba8 && !bgra8){
                //Exotic format: fall back to the generic unpack, point sampled. This path is
                //not expected on a window buffer and is not worth optimising.
                for(uint32_t y = 0; y < dstHeight; y++){
                    const uint32_t srcY = static_cast<uint32_t>((y + 0.5f) * height / dstHeight);
                    for(uint32_t x = 0; x < dstWidth; x++){
                        const uint32_t srcX = static_cast<uint32_t>((x + 0.5f) * width / dstWidth);
                        const Ogre::ColourValue colour = box.getColourAt(
                            std::min(srcX, width - 1u), std::min(srcY, height - 1u), 0, format);
                        uint8_t* dst = &out.rgb[(static_cast<size_t>(y) * dstWidth + x) * 3];
                        dst[0] = static_cast<uint8_t>(std::max(0.0f, std::min(1.0f, colour.r)) * 255.0f + 0.5f);
                        dst[1] = static_cast<uint8_t>(std::max(0.0f, std::min(1.0f, colour.g)) * 255.0f + 0.5f);
                        dst[2] = static_cast<uint8_t>(std::max(0.0f, std::min(1.0f, colour.b)) * 255.0f + 0.5f);
                    }
                }
                sLastUnpackUs = std::chrono::duration<double, std::micro>(
                    std::chrono::steady_clock::now() - unpackStart).count();
                return true;
            }

            const uint32_t rIdx = bgra8 ? 2 : 0;
            const uint32_t bIdx = bgra8 ? 0 : 2;

            if(fastSample){
                //Touches one source pixel per output pixel, so the cost is the size of the
                //target rather than the size of the window.
                for(uint32_t y = 0; y < dstHeight; y++){
                    const uint32_t srcY = std::min(static_cast<uint32_t>((y + 0.5f) * height / dstHeight), height - 1u);
                    const uint8_t* row = reinterpret_cast<const uint8_t*>(box.at(0, srcY, 0));
                    uint8_t* dst = &out.rgb[static_cast<size_t>(y) * dstWidth * 3];
                    for(uint32_t x = 0; x < dstWidth; x++){
                        const uint32_t srcX = std::min(static_cast<uint32_t>((x + 0.5f) * width / dstWidth), width - 1u);
                        const uint8_t* src = row + static_cast<size_t>(srcX) * 4;
                        dst[0] = src[rIdx];
                        dst[1] = src[1];
                        dst[2] = src[bIdx];
                        dst += 3;
                    }
                }
                sLastUnpackUs = std::chrono::duration<double, std::micro>(
                    std::chrono::steady_clock::now() - unpackStart).count();
                return true;
            }

            //Box average, one sequential pass over the source. The column each source pixel
            //belongs to is precomputed, because a division per pixel costs more than the
            //accumulation itself.
            std::vector<uint32_t> columnOf(width);
            for(uint32_t x = 0; x < width; x++){
                columnOf[x] = std::min(x * dstWidth / width, dstWidth - 1u);
            }

            std::vector<uint32_t> accum(static_cast<size_t>(dstWidth) * 3, 0);
            std::vector<uint32_t> counts(dstWidth, 0);
            uint32_t currentRow = 0;

            const auto flushRow = [&](uint32_t row){
                uint8_t* dst = &out.rgb[static_cast<size_t>(row) * dstWidth * 3];
                for(uint32_t x = 0; x < dstWidth; x++){
                    const uint32_t count = counts[x] ? counts[x] : 1u;
                    dst[x * 3 + 0] = static_cast<uint8_t>(accum[x * 3 + 0] / count);
                    dst[x * 3 + 1] = static_cast<uint8_t>(accum[x * 3 + 1] / count);
                    dst[x * 3 + 2] = static_cast<uint8_t>(accum[x * 3 + 2] / count);
                }
                std::fill(accum.begin(), accum.end(), 0u);
                std::fill(counts.begin(), counts.end(), 0u);
            };

            for(uint32_t y = 0; y < height; y++){
                const uint32_t row = std::min(y * dstHeight / height, dstHeight - 1u);
                if(row != currentRow){
                    flushRow(currentRow);
                    currentRow = row;
                }

                const uint8_t* src = reinterpret_cast<const uint8_t*>(box.at(0, y, 0));
                for(uint32_t x = 0; x < width; x++){
                    const uint32_t column = columnOf[x];
                    accum[column * 3 + 0] += src[rIdx];
                    accum[column * 3 + 1] += src[1];
                    accum[column * 3 + 2] += src[bIdx];
                    counts[column]++;
                    src += 4;
                }
            }
            flushRow(currentRow);

            sLastUnpackUs = std::chrono::duration<double, std::micro>(
                std::chrono::steady_clock::now() - unpackStart).count();
        }catch(Ogre::Exception& e){
            AV_ERROR("FrameCapture downsampled readback failed: {}", e.getDescription());
            outError = "readback failed: " + e.getDescription();
            return false;
        }

        return true;
    }
}

#endif
