#include "OgreHlmsPbsAVCustom.h"

#include "System/EnginePrerequisites.h"
#include "OgreHlmsManager.h"

#include "OgreHlmsPbsDatablock.h"

namespace Ogre{

    HlmsPbsAVCustom::HlmsPbsAVCustom(Archive *dataFolder, ArchiveVec *libraryFolders) : Ogre::HlmsPbs(dataFolder, libraryFolders){

    }

    void HlmsPbsAVCustom::calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces ){
        for(HlmsAVCustomListener* listener : mAVCustomListeners){
            listener->calculateHashForPreCreate(this, renderable, inOutPieces);
        }

        HlmsPbs::calculateHashForPreCreate(renderable, inOutPieces);
    }

    void HlmsPbsAVCustom::calculateHashForPreCaster( Renderable *renderable, PiecesMap *inOutPieces, const PiecesMap *normalPassPieces ){
        //Pre caster will clear properties, so it needs to be called before the for loop.
        HlmsPbs::calculateHashForPreCaster(renderable, inOutPieces, normalPassPieces);

        for(HlmsAVCustomListener* listener : mAVCustomListeners){
            listener->calculateHashForPreCaster(this, renderable, inOutPieces, normalPassPieces);
        }
    }

    Ogre::uint32 HlmsPbsAVCustom::fillBuffersForV2(const Ogre::HlmsCache *cache, const Ogre::QueuedRenderable &queuedRenderable, bool casterPass, Ogre::uint32 lastCacheHash, Ogre::CommandBuffer *commandBuffer){
        Ogre::uint32 result =
        HlmsPbs::fillBuffersForV2(cache, queuedRenderable, casterPass, lastCacheHash, commandBuffer);

        for(HlmsAVCustomListener* listener : mAVCustomListeners){
            listener->fillBuffersForV2(cache, queuedRenderable, casterPass, lastCacheHash, commandBuffer);
        }

        return result;
    }

    void HlmsPbsAVCustom::registerCustomListener(HlmsAVCustomListener* listener){
        mAVCustomListeners.push_back(listener);
    }

    void HlmsPbsAVCustom::setProperty(IdString key, int32 value){
        Hlms::setProperty(key, value);
    }

}
