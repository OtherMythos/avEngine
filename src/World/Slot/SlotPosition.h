#pragma once

#include "OgreVector3.h"

namespace AV {
    class SlotPosition{
    public:
        int chunkX, chunkY;
        Ogre::Vector3 position;
        
        bool operator==(const SlotPosition &pos) const;
        SlotPosition& operator=(const SlotPosition &pos) ;
        
    public:
        SlotPosition();
        SlotPosition(int chunkX, int chunkY, Ogre::Vector3 position);
        
        Ogre::Vector3 toOgre() const;
    };
}
