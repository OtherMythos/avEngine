#ifdef FLIGHT_RECORDER

#include "RecorderFrameListener.h"

#include "FlightRecorder.h"
#include "System/Capture/FrameCapture.h"
#include "Logger/Log.h"

#include "Window/Window.h"
#include "System/BaseSingleton.h"

#include "OgreRoot.h"
#include "OgreWindow.h"
#include "OgreTextureGpu.h"
#include "OgrePixelFormatGpuUtils.h"

#include <algorithm>
#include <string>

namespace AV{

    void RecorderFrameListener::initialise(uint32_t captureWidth, uint32_t captureHeight, uint32_t everyNthFrame){
        mCaptureWidth = captureWidth;
        mCaptureHeight = captureHeight;
        mEveryNthFrame = everyNthFrame > 0 ? everyNthFrame : 1;

        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        if(!root) return;
        root->addFrameListener(this);
        mRegistered = true;
    }

    void RecorderFrameListener::shutdown(){
        mAsyncReader.shutdown();

        if(!mRegistered) return;
        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        if(root) root->removeFrameListener(this);
        mRegistered = false;
    }

    bool RecorderFrameListener::frameRenderingQueued(const Ogre::FrameEvent& evt){
        mFrameNumber++;

        if(!FlightRecorder::isRunning()) return true;

        //Nothing has rendered into the window yet - a project with a custom compositor
        //creates its workspace from script, a few frames in - so the colour buffer has no
        //drawable and reading it back would segfault inside the driver rather than fail
        //gracefully. Skip the frame; recording begins as soon as there is something to record.
        if(!FlightRecorder::windowHasWorkspace()) return true;

        const bool wantFull = mWantFullFrame;
        //Sampling never skips the frame a capture landed on: that is the one frame the
        //player actually pointed at.
        if(!wantFull && (mFrameNumber % mEveryNthFrame) != 0) return true;

        CapturedFrame full;
        std::string error;

        //Set up the non stalling path on the first recorded frame, once the render texture
        //exists and its real format is known.
        Ogre::TextureGpu* texture = 0;
        Window* window = BaseSingleton::getWindow();
        if(window) texture = window->getRenderTexture();

        if(!mAsyncAttempted && texture){
            mAsyncAttempted = true;
            //Only an offscreen target can be read back asynchronously. A window's swapchain
            //texture cannot be blitted out of mid frame - it segfaults inside the Metal
            //driver - so a windowed run uses the synchronous path that Ogre supports for a
            //drawable. See AsyncFrameReader's header.
            const bool windowSpecific = texture->isRenderWindowSpecific();
            const bool ok = !windowSpecific && mAsyncReader.initialise(texture->getWidth(), texture->getHeight(),
                Ogre::PixelFormatGpuUtils::getFamily(texture->getPixelFormat()));
            AV_INFO("Flight recorder readback mode: {}", ok
                ? "asynchronous"
                : (windowSpecific ? "synchronous (window swapchain cannot be read back asynchronously)"
                                  : "synchronous (fallback)"));
        }

        bool haveFrame = false;
        if(mAsyncReader.isReady() && texture){
            //Returns nothing for the first few frames while the pool fills, which is normal.
            haveFrame = mAsyncReader.submitAndCollect(texture, mFrameNumber, full, error);
            if(!haveFrame && !error.empty()){
                if(!mLoggedFailure){
                    AV_ERROR("Flight recorder async readback failed, falling back to synchronous capture: {}", error);
                    mLoggedFailure = true;
                }
                mAsyncReader.shutdown();
            }
            //A full resolution frame is wanted now, but the async path can only deliver one
            //that is a few frames old. Take the synchronous hit for this one frame so the
            //capture frame is the one the player was actually looking at.
            if(wantFull){
                CapturedFrame sync;
                std::string syncError;
                if(FrameCapture::readColourBuffer(sync, syncError)){
                    sync.frameNumber = mFrameNumber;
                    mWantFullFrame = false;
                    FlightRecorder::_notifyFullFrame(std::move(sync));
                }
            }
            if(!haveFrame) return true;
        }else{
            if(!FrameCapture::readColourBuffer(full, error)){
                if(!mLoggedFailure){
                    AV_ERROR("Flight recorder frame readback failed, no further frames will be recorded this session: {}", error);
                    mLoggedFailure = true;
                }
                return true;
            }
            full.frameNumber = mFrameNumber;

            if(wantFull){
                mWantFullFrame = false;
                CapturedFrame copy = full;
                FlightRecorder::_notifyFullFrame(std::move(copy));
            }
        }

        //Fit the frame inside the configured box rather than stretching it to fill it.
        //boxDownsample clamps each dimension independently, so passing the box directly
        //would squash a window whose aspect ratio differs from it, and a distorted frame is
        //actively misleading to someone diagnosing a visual bug.
        uint32_t outWidth = mCaptureWidth;
        uint32_t outHeight = mCaptureHeight;
        if(full.width > 0 && full.height > 0){
            const double scale = std::min(static_cast<double>(mCaptureWidth) / full.width,
                                          static_cast<double>(mCaptureHeight) / full.height);
            if(scale < 1.0){
                outWidth = std::max<uint32_t>(1, static_cast<uint32_t>(full.width * scale + 0.5));
                outHeight = std::max<uint32_t>(1, static_cast<uint32_t>(full.height * scale + 0.5));
            }else{
                //Already smaller than the box; keep it as it is rather than upsampling.
                outWidth = full.width;
                outHeight = full.height;
            }
        }

        CapturedFrame small = ImageOps::boxDownsample(full, outWidth, outHeight);
        //From full, not mFrameNumber: on the async path the frame collected was queued
        //several frames ago and must keep the number it was rendered as.
        small.frameNumber = full.frameNumber;

        FlightRecorder::_notifyFrameCaptured(std::move(small));
        return true;
    }
}

#endif
