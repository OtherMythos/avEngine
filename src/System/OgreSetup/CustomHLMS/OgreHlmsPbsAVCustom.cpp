#include "OgreHlmsPbsAVCustom.h"

#include "System/EnginePrerequisites.h"
#include "OgreHlmsManager.h"

#include "OgreHlmsPbsDatablock.h"

namespace Ogre{

    HlmsPbsAVCustom::HlmsPbsAVCustom(Archive *dataFolder, ArchiveVec *libraryFolders) : Ogre::HlmsPbs(dataFolder, libraryFolders){

    }

    void HlmsPbsAVCustom::calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces ){
        HlmsPbs::calculateHashForPreCreate(renderable, inOutPieces);

        for(HlmsAVCustomListener* listener : mAVCustomListeners){
            listener->calculateHashForPreCreate(this, renderable, inOutPieces);
        }
    }

    void HlmsPbsAVCustom::calculateHashForPreCaster( Renderable *renderable, PiecesMap *inOutPieces, const PiecesMap *normalPassPieces ){
        HlmsPbs::calculateHashForPreCaster(renderable, inOutPieces, normalPassPieces);

        for(HlmsAVCustomListener* listener : mAVCustomListeners){
            listener->calculateHashForPreCaster(this, renderable, inOutPieces, normalPassPieces);
        }
    }

    void HlmsPbsAVCustom::registerCustomListener(HlmsAVCustomListener* listener){
        mAVCustomListeners.push_back(listener);
    }

    void HlmsPbsAVCustom::setProperty(IdString key, int32 value){
        Hlms::setProperty(key, value);
    }

}
