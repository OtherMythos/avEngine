#include "OgreHlmsUnlitAVCustom.h"

#include "System/EnginePrerequisites.h"
#include "OgreHlmsManager.h"

namespace Ogre{

    HlmsUnlitAVCustom::HlmsUnlitAVCustom(Archive *dataFolder, ArchiveVec *libraryFolders) : Ogre::HlmsColibri(dataFolder, libraryFolders){

    }

    void HlmsUnlitAVCustom::calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces ){
        for(HlmsUnlitAVCustomListener* listener : mAVCustomListeners){
            listener->calculateHashForPreCreate(this, renderable, inOutPieces);
        }

        HlmsColibri::calculateHashForPreCreate(renderable, inOutPieces);
    }

    void HlmsUnlitAVCustom::calculateHashForPreCaster( Renderable *renderable, PiecesMap *inOutPieces, const PiecesMap *normalPassPieces ){
        //Pre caster will clear properties, so it needs to be called before the for loop.
        HlmsColibri::calculateHashForPreCaster(renderable, inOutPieces, normalPassPieces);

        for(HlmsUnlitAVCustomListener* listener : mAVCustomListeners){
            listener->calculateHashForPreCaster(this, renderable, inOutPieces, normalPassPieces);
        }
    }

    Ogre::uint32 HlmsUnlitAVCustom::fillBuffersForV2(const Ogre::HlmsCache *cache, const Ogre::QueuedRenderable &queuedRenderable, bool casterPass, Ogre::uint32 lastCacheHash, Ogre::CommandBuffer *commandBuffer){
        Ogre::uint32 result =
        HlmsColibri::fillBuffersForV2(cache, queuedRenderable, casterPass, lastCacheHash, commandBuffer);

        for(HlmsUnlitAVCustomListener* listener : mAVCustomListeners){
            listener->fillBuffersForV2(cache, queuedRenderable, casterPass, lastCacheHash, commandBuffer);
        }

        return result;
    }

    void HlmsUnlitAVCustom::registerCustomListener(HlmsUnlitAVCustomListener* listener){
        mAVCustomListeners.push_back(listener);
    }

    void HlmsUnlitAVCustom::setProperty(IdString key, int32 value){
        Hlms::setProperty(key, value);
    }

}
