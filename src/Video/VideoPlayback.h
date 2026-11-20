#pragma once

#include "MpegVideo.h"

#include <memory>
#include <string>

namespace Ogre{
    class RenderSystem;
    class TextureGpu;
    class TextureGpuManager;
}

namespace AV{
    class VideoPlayback{
    public:
        struct FrameTimings{
            double decodeMs = 0.0;
            double uploadMs = 0.0;
        };

        explicit VideoPlayback(const std::string& path);

        bool advance(double seconds);
        void rewind();
        bool hasEnded() const { return mVideo.hasEnded(); }
        double getFrameRate() const { return mVideo.getFrameRate(); }
        double getPixelAspectRatio() const { return mVideo.getPixelAspectRatio(); }
        Ogre::TextureGpu* getTexture() const { return mTexture.get(); }
        FrameTimings getLastFrameTimings() const { return mLastFrameTimings; }

    private:
        struct DestroyTexture{
            void operator()(Ogre::TextureGpu* texture) const;
        };

        void upload();

        MpegVideo mVideo;
        Ogre::RenderSystem* mRenderSystem;
        Ogre::TextureGpuManager* mTextureManager;
        std::unique_ptr<Ogre::TextureGpu, DestroyTexture> mTexture;
        FrameTimings mLastFrameTimings;
    };
}
