#pragma once

#include "OgreVector3.h"

namespace AV {
    /**
     An abstraction of the world coordinate space.
     Coordinates are given as SlotPositions rather than absolute positions, so that when the origin is shifted the correct absolute position can be determined.

     The class doesn't allow direct access to the position and chunk variables because they need to be validated.
     For instance, the chunkX and Y can't go below 0, because that's not allowed as part of the coordinate system.
     Neither is a position less than 0 or greater than the slot size.
     These values are checked when set, and will be set to acceptable values if necessary.
     */
    class SlotPosition{
    private:
        int _chunkX, _chunkY;
        Ogre::Vector3 _position;
    public:
        bool operator==(const SlotPosition &pos) const;
        SlotPosition& operator=(const SlotPosition &pos);

        SlotPosition operator+(const SlotPosition &pos);
        SlotPosition operator-(const SlotPosition &pos);

        SlotPosition operator+(const Ogre::Vector3 &ammount);
        SlotPosition operator-(const Ogre::Vector3 &ammount);

        friend std::ostream& operator << (std::ostream& o, const SlotPosition &coord);

        int chunkX() const { return _chunkX; };
        int chunkY() const { return _chunkY; };
        Ogre::Vector3 position() const { return _position; };

    public:
        SlotPosition();
        SlotPosition(int chunkX, int chunkY, Ogre::Vector3 position);
        SlotPosition(int chunkX, int chunkY);
        SlotPosition(const SlotPosition &pos);

        /**
        Create a slot position from an ogre position relative to the origin.
        */
        SlotPosition(const Ogre::Vector3 &pos);

        /**
         Convert the slot position into an ogre vector3 relative to the origin.

         @return
         An Ogre vector3 relative to the origin.
         */
        Ogre::Vector3 toOgre() const;
    };
}
