#pragma once

#include "OgreHlmsPbs.h"

#include <vector>

namespace Ogre{

    class HlmsAVCustomListener;

    class HlmsPbsAVCustom : public Ogre::HlmsPbs{
    public:

        HlmsPbsAVCustom( Archive *dataFolder, ArchiveVec *libraryFolders );

        void calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces ) override;

        void registerCustomListener(HlmsAVCustomListener* listener);

        void setProperty(IdString key, int32 value);

    private:
        std::vector<HlmsAVCustomListener*> mAVCustomListeners;

    };

    class HlmsAVCustomListener{
    public:
        virtual void calculateHashForPreCreate( Ogre::HlmsPbsAVCustom* hlms, Renderable *renderable, PiecesMap *inOutPieces ) = 0;
    };

};
