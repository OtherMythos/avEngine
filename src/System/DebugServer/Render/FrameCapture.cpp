#ifdef DEBUG_SERVER

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

#include <algorithm>
#include <chrono>

namespace AV{
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
        Window* window = BaseSingleton::getWindow();
        if(!window){
            mError = "window not available";
            return false;
        }
        Ogre::Window* renderWindow = window->getRenderWindow();
        if(!renderWindow){
            mError = "render window not available";
            return false;
        }
        Ogre::TextureGpu* texture = renderWindow->getTexture();
        if(!texture){
            mError = "window texture not available";
            return false;
        }

        try{
            //Synchronous GPU->CPU readback. automaticResolve handles MSAA window buffers.
            Ogre::Image2 image;
            image.convertFromTexture(texture, 0u, 0u, true);

            const uint32_t width = image.getWidth();
            const uint32_t height = image.getHeight();
            if(width == 0 || height == 0){
                mError = "captured image is empty";
                return false;
            }

            mFrame.width = width;
            mFrame.height = height;
            mFrame.frameNumber = mFrameNumber;
            mFrame.rgb.resize(static_cast<size_t>(width) * height * 3);

            //The stored bytes of a UNORM_SRGB window buffer are display-referred, which is
            //what an agent asking "what is on screen" wants.
            Ogre::TextureBox box = image.getData(0);
            const Ogre::PixelFormatGpu format = image.getPixelFormat();
            uint8_t* dst = mFrame.rgb.data();

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
        }catch(Ogre::Exception& e){
            AV_ERROR("FrameCapture readback failed: {}", e.getDescription());
            mError = "readback failed: " + e.getDescription();
            return false;
        }

        return true;
    }
}

#endif
