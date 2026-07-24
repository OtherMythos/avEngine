#pragma once

#include "ColibriGui/Ogre/OgreHlmsColibri.h"

#include <vector>

namespace Ogre{

    class HlmsUnlitAVCustomListener;

    class HlmsUnlitAVCustom : public Ogre::HlmsColibri{
    public:

        HlmsUnlitAVCustom( Archive *dataFolder, ArchiveVec *libraryFolders );

        void calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces ) override;
        void calculateHashForPreCaster( Renderable *renderable, PiecesMap *inOutPieces, const PiecesMap *normalPassPieces ) override;
        Ogre::uint32 fillBuffersForV2(const Ogre::HlmsCache *cache, const Ogre::QueuedRenderable &queuedRenderable, bool casterPass, Ogre::uint32 lastCacheHash, Ogre::CommandBuffer *commandBuffer) override;

        void registerCustomListener(HlmsUnlitAVCustomListener* listener);

        void setProperty(IdString key, int32 value);

        //Colibri creates the glyph atlas lazily, on the first rasterised glyph, and hands it to
        //us here. Until that happens none of it exists on the GPU.
        bool hasGlyphAtlasBuffer() const { return mGlyphAtlasBuffer != 0; }

    private:
        std::vector<HlmsUnlitAVCustomListener*> mAVCustomListeners;

    };

    class HlmsUnlitAVCustomListener{
    public:
        virtual void calculateHashForPreCreate( Ogre::HlmsUnlitAVCustom* hlms, Renderable *renderable, PiecesMap *inOutPieces ) = 0;
        virtual void calculateHashForPreCaster( Ogre::HlmsUnlitAVCustom* hlms, Ogre::Renderable *renderable, PiecesMap *inOutPieces, const PiecesMap *normalPassPieces ) = 0;
        virtual Ogre::uint32 fillBuffersForV2(const Ogre::HlmsCache *cache, const Ogre::QueuedRenderable &queuedRenderable, bool casterPass, Ogre::uint32 lastCacheHash, Ogre::CommandBuffer *commandBuffer) = 0;
    };

};
