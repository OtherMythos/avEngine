#pragma once

#include "OgreVector3.h"
#include <LinearMath/btVector3.h>

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

        void _minusOperation(const Ogre::Vector3 &amount, SlotPosition& pos) const;
        void _additionOperation(const Ogre::Vector3 &amount, SlotPosition& pos) const;

        Ogre::Vector3 _ogreToOrigin(const SlotPosition& origin) const;
    public:
        static const SlotPosition ZERO;

        bool operator<(const SlotPosition &pos) const;
        bool operator==(const SlotPosition &pos) const;
        SlotPosition& operator=(const SlotPosition &pos);

        SlotPosition operator+(const SlotPosition &pos) const;
        SlotPosition operator-(const SlotPosition &pos) const;

        SlotPosition operator+(const Ogre::Vector3 &ammount) const;
        SlotPosition operator-(const Ogre::Vector3 &ammount) const;

        SlotPosition operator+(const btVector3 &ammount) const;
        SlotPosition operator-(const btVector3 &ammount) const;

        void operator+=(const SlotPosition &pos);
        void operator-=(const SlotPosition &pos);
        void operator+=(const Ogre::Vector3& vec);
        void operator-=(const Ogre::Vector3& vec);
        void operator+=(const btVector3& vec);
        void operator-=(const btVector3& vec);

        friend std::ostream& operator << (std::ostream& o, const SlotPosition &coord);
        void printLeanStream(std::ostream& o) const;

        int chunkX() const { return _chunkX; };
        int chunkY() const { return _chunkY; };
        const Ogre::Vector3& position() const { return _position; };

    public:
        SlotPosition();
        SlotPosition(int chunkX, int chunkY, Ogre::Vector3 position);
        SlotPosition(int chunkX, int chunkY);
        SlotPosition(const SlotPosition &pos);
        SlotPosition(const std::string& s);

        /**
        Create a slot position from an ogre position relative to the origin.
        */
        SlotPosition(const Ogre::Vector3 &pos);

        /**
        Create a slot position from a bullet vector relative to the origin.
        */
        SlotPosition(const btVector3 &pos);

        /**
         Convert the slot position into an ogre vector3 relative to the origin.

         @return
         An Ogre vector3 relative to the origin.
         */
        Ogre::Vector3 toOgre() const;


        /**
        Convert the Slot Position into an Ogre Vector3, assuming the origin is 0, 0, 0.
        Useful for determining offsets.
        */
        Ogre::Vector3 toOgreAbsolute() const;

        /**
        Move the slot position towards the destination.
        */
        void moveTowards(const SlotPosition &destination, Ogre::Real interval);

        Ogre::Vector3 toOgreWithOrigin(const SlotPosition& origin) const;

        Ogre::Real getDistanceBetween(const SlotPosition& pos) const;

        /**
         Convert the slot position into a bullet vector3 relative to the origin.

         @return
         A bullet vector3 relative to the origin.
         */
        btVector3 toBullet() const;

        /**
        Convert a vector to bullet using a provided origin.
        This can be useful to create a threadsafe function, as there is now no need to read from the main thread's stored origin.
        */
        btVector3 toBulletWithOrigin(const SlotPosition& origin) const;
    };
}
