#ifdef FLIGHT_RECORDER

#include "AsyncFrameReader.h"

#include "Logger/Log.h"

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreTextureGpu.h"
#include "OgreTextureGpuManager.h"
#include "OgreAsyncTextureTicket.h"
#include "OgreTextureBox.h"
#include "OgrePixelFormatGpuUtils.h"
#include "OgreException.h"

#include <algorithm>

namespace AV{

    AsyncFrameReader::~AsyncFrameReader(){
        shutdown();
    }

    static Ogre::TextureGpuManager* textureManager(){
        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        if(!root) return 0;
        Ogre::RenderSystem* renderSystem = root->getRenderSystem();
        if(!renderSystem) return 0;
        return renderSystem->getTextureGpuManager();
    }

    bool AsyncFrameReader::initialise(uint32_t width, uint32_t height, Ogre::PixelFormatGpu format){
        shutdown();

        Ogre::TextureGpuManager* manager = textureManager();
        if(!manager || width == 0 || height == 0) return false;

        mWidth = width;
        mHeight = height;
        mFormat = format;

        try{
            mSlots.resize(POOL_SIZE);
            for(Slot& slot : mSlots){
                slot.ticket = manager->createAsyncTextureTicket(width, height, 1u,
                    Ogre::TextureTypes::Type2D, format);
                if(!slot.ticket){
                    shutdown();
                    return false;
                }
            }
        }catch(Ogre::Exception& e){
            AV_WARN("Flight recorder could not create async readback tickets, falling back to synchronous capture: {}", e.getDescription());
            shutdown();
            return false;
        }

        mNext = 0;
        mReady = true;
        return true;
    }

    void AsyncFrameReader::shutdown(){
        Ogre::TextureGpuManager* manager = textureManager();
        for(Slot& slot : mSlots){
            if(!slot.ticket) continue;
            //A ticket mid transfer must still be destroyed through the manager that made it.
            if(manager) manager->destroyAsyncTextureTicket(slot.ticket);
            slot.ticket = nullptr;
        }
        mSlots.clear();
        mNext = 0;
        mReady = false;
    }

    bool AsyncFrameReader::submitAndCollect(Ogre::TextureGpu* texture, uint64_t frameNumber,
                                            CapturedFrame& out, std::string& outError){
        if(!mReady || !texture){
            outError = "async reader not ready";
            return false;
        }

        //The window may have been resized since the pool was built, in which case the ticket
        //dimensions no longer match and the pool has to be rebuilt.
        if(texture->getWidth() != mWidth || texture->getHeight() != mHeight){
            if(!initialise(texture->getWidth(), texture->getHeight(),
                           Ogre::PixelFormatGpuUtils::getFamily(texture->getPixelFormat()))){
                outError = "could not rebuild the ticket pool after a resize";
                return false;
            }
        }

        Slot& slot = mSlots[mNext];
        mNext = (mNext + 1) % mSlots.size();

        bool collected = false;
        //This slot's download was queued POOL_SIZE frames ago, so the transfer has had that
        //long to finish and mapping it should not stall.
        if(slot.pending){
            try{
                const Ogre::TextureBox box = slot.ticket->map(0);
                _copyOut(box, mFormat, mWidth, mHeight, out);
                slot.ticket->unmap();
                out.frameNumber = slot.frameNumber;
                collected = true;
            }catch(Ogre::Exception& e){
                outError = "async map failed: " + e.getDescription();
            }
            slot.pending = false;
        }

        try{
            slot.ticket->download(texture, 0u, false);
            slot.pending = true;
            slot.frameNumber = frameNumber;
        }catch(Ogre::Exception& e){
            outError = "async download failed: " + e.getDescription();
            slot.pending = false;
        }

        return collected;
    }

    void AsyncFrameReader::_copyOut(const Ogre::TextureBox& box, Ogre::PixelFormatGpu format,
                                    uint32_t width, uint32_t height, CapturedFrame& out){
        out.width = width;
        out.height = height;
        out.rgb.resize(static_cast<size_t>(width) * height * 3);

        uint8_t* dst = out.rgb.data();

        //Same fast paths as FrameCapture: window buffers are almost always 8 bit rgba or
        //bgra, and the generic per pixel unpack is far too slow at these sizes.
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
    }
}

#endif
