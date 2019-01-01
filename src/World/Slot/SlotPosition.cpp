#include "SlotPosition.h"

namespace AV{
    SlotPosition::SlotPosition()
        : chunkX(0),
        chunkY(0),
        position(Ogre::Vector3::ZERO){
        
    }

    SlotPosition::SlotPosition(int chunkX, int chunkY, Ogre::Vector3 position)
        : chunkX(chunkX),
        chunkY(chunkY),
        position(position){
        
    }
    
    bool SlotPosition::operator==(const SlotPosition &pos) const{
        if(pos.chunkX == chunkX && pos.chunkY == chunkY && pos.position == position) return true;
        else return false;
    }
    
    SlotPosition& SlotPosition::operator=(const SlotPosition &pos){
        chunkX = pos.chunkX;
        chunkY = pos.chunkY;
        position = pos.position;
        
        return *this;
    }
}
