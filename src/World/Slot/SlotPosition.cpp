#include "SlotPosition.h"

#include "World/WorldSingleton.h"
#include "System/SystemSetup/SystemSettings.h"

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
    
    Ogre::Vector3 SlotPosition::toOgre() const{
        const SlotPosition& origin = WorldSingleton::getOrigin();
        int slotSize = SystemSettings::getWorldSlotSize();
        
        int offsetX = chunkX - origin.chunkX;
        int offsetY = chunkY - origin.chunkY;
        Ogre::Vector3 posDiff = position - origin.position;
        Ogre::Vector3 dest = Ogre::Vector3(offsetX * slotSize, 0, offsetY * slotSize) + posDiff;
        
        return dest;
    }
}
