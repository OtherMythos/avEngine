#include "VideoPlayback.h"

#include "OgreRenderSystem.h"
#include "OgreRoot.h"
#include "OgreStagingTexture.h"
#include "OgreTextureGpuManager.h"

#include <chrono>
#include <stdexcept>

namespace AV{
    void VideoPlayback::DestroyTexture::operator()(Ogre::TextureGpu* texture) const{
        texture->getTextureManager()->destroyTexture(texture);
    }

    VideoPlayback::VideoPlayback(const std::string& path)
        : mVideo(path),
          mRenderSystem(Ogre::Root::getSingleton().getRenderSystem()),
          mTextureManager(mRenderSystem->getTextureGpuManager()){
        static unsigned nextId = 0;
        const auto name = "/video/mpeg1/" + std::to_string(nextId++);
        mTexture.reset(mTextureManager->createTexture(name, Ogre::GpuPageOutStrategy::Discard,
                                                       Ogre::TextureFlags::ManualTexture, Ogre::TextureTypes::Type2D));
        mTexture->setResolution(mVideo.getWidth(), mVideo.getHeight());
        mTexture->setPixelFormat(Ogre::PFG_RGBA8_UNORM_SRGB);
        mTexture->scheduleTransitionTo(Ogre::GpuResidency::Resident);
        upload();
    }

    bool VideoPlayback::advance(double seconds){
        using Clock = std::chrono::steady_clock;
        const auto start = Clock::now();
        if(!mVideo.advance(seconds)) return false;

        const auto decoded = Clock::now();
        upload();
        mLastFrameTimings.decodeMs = std::chrono::duration<double, std::milli>(decoded - start).count();
        mLastFrameTimings.uploadMs = std::chrono::duration<double, std::milli>(Clock::now() - decoded).count();
        return true;
    }

    void VideoPlayback::rewind(){
        mVideo.rewind();
        upload();
    }

    void VideoPlayback::upload(){
        const auto width = mTexture->getWidth();
        const auto height = mTexture->getHeight();
        const auto format = mTexture->getPixelFormat();
        auto* staging = mTextureManager->getStagingTexture(width, height, 1, 1, format);
        try{
            staging->startMapRegion();
            const auto box = staging->mapRegion(width, height, 1, 1, format);
            if(box.data){
                mVideo.writeRgba(static_cast<std::uint8_t*>(box.data), box.bytesPerRow);
            }
            staging->stopMapRegion();
            if(!box.data) throw std::runtime_error("Cannot map MPEG-1 video staging texture.");
            staging->upload(box, mTexture.get(), 0);
        }catch(...){
            mTextureManager->removeStagingTexture(staging);
            throw;
        }
        mTextureManager->removeStagingTexture(staging);

        auto& solver = mRenderSystem->getBarrierSolver();
        auto& transitions = solver.getNewResourceTransitionsArrayTmp();
        mRenderSystem->endCopyEncoder();
        solver.resolveTransition(transitions, mTexture.get(), Ogre::ResourceLayout::Texture,
                                 Ogre::ResourceAccess::Read, 1u << Ogre::GPT_FRAGMENT_PROGRAM);
        mRenderSystem->executeResourceTransition(transitions);
    }
}
