#pragma once

#include "OgreVector3.h"

namespace AV {
    /**
     An abstraction of the world coordinate space.
     Coordinates are given as SlotPositions rather than absolute positions, so that when the origin is shifted the correct absolute position can be determined.
     */
    class SlotPosition{
    public:
        int chunkX, chunkY;
        Ogre::Vector3 position;
        
        bool operator==(const SlotPosition &pos) const;
        SlotPosition& operator=(const SlotPosition &pos) ;
        
    public:
        SlotPosition();
        SlotPosition(int chunkX, int chunkY, Ogre::Vector3 position);
        SlotPosition(int chunkX, int chunkY);
        
        /**
         Convert the slot position into an ogre vector3 relative to the origin.
         
         @return
         An Ogre vector3 relative to the origin.
         */
        Ogre::Vector3 toOgre() const;
    };
}
