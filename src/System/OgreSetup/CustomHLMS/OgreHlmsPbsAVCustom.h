#pragma once

#include "OgreHlmsPbs.h"

#include <vector>

namespace Ogre{

    class HlmsAVCustomListener;

    class HlmsPbsAVCustom : public Ogre::HlmsPbs{
    public:

        HlmsPbsAVCustom( Archive *dataFolder, ArchiveVec *libraryFolders );

        void calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces ) override;
        void calculateHashForPreCaster( Renderable *renderable, PiecesMap *inOutPieces, const PiecesMap *normalPassPieces ) override;
        Ogre::uint32 fillBuffersForV2(const Ogre::HlmsCache *cache, const Ogre::QueuedRenderable &queuedRenderable, bool casterPass, Ogre::uint32 lastCacheHash, Ogre::CommandBuffer *commandBuffer) override;

        void registerCustomListener(HlmsAVCustomListener* listener);

        void setProperty(IdString key, int32 value);

    private:
        std::vector<HlmsAVCustomListener*> mAVCustomListeners;

    };

    class HlmsAVCustomListener{
    public:
        virtual void calculateHashForPreCreate( Ogre::HlmsPbsAVCustom* hlms, Renderable *renderable, PiecesMap *inOutPieces ) = 0;
        virtual void calculateHashForPreCaster( Ogre::HlmsPbsAVCustom* hlms, Ogre::Renderable *renderable, PiecesMap *inOutPieces, const PiecesMap *normalPassPieces ) = 0;
        virtual Ogre::uint32 fillBuffersForV2(const Ogre::HlmsCache *cache, const Ogre::QueuedRenderable &queuedRenderable, bool casterPass, Ogre::uint32 lastCacheHash, Ogre::CommandBuffer *commandBuffer) = 0;
    };

};
