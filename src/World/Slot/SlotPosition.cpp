#include "SlotPosition.h"

#include "World/WorldSingleton.h"
#include "System/SystemSetup/SystemSettings.h"

namespace AV{
    SlotPosition::SlotPosition()
        : _chunkX(0),
        _chunkY(0),
        _position(Ogre::Vector3::ZERO){

    }

    SlotPosition::SlotPosition(int cX, int cY, Ogre::Vector3 pos)
        : _chunkX(cX),
        _chunkY(cY),
        _position(pos){
            int slotSize = SystemSettings::getWorldSlotSize();
            if(_position.x > slotSize) _position.x = slotSize;
            if(_position.z > slotSize) _position.z = slotSize;
            if(_position.x < 0) _position.x = 0;
            if(_position.z < 0) _position.z = 0;
    }

    SlotPosition::SlotPosition(int chunkX, int chunkY)
    : _chunkX(chunkX),
    _chunkY(chunkY),
    _position(Ogre::Vector3::ZERO){

    }

    SlotPosition::SlotPosition(const SlotPosition &pos)
    :_chunkX(pos.chunkX()),
    _chunkY(pos.chunkY()),
    _position(pos.position()){

    }

    SlotPosition::SlotPosition(const Ogre::Vector3 &pos){
        int slotSize = SystemSettings::getWorldSlotSize();
        const SlotPosition& origin = WorldSingleton::getOrigin();

        //Create a basic slot position from that position.
        int chunkX = pos.x / slotSize;
        int chunkY = pos.z / slotSize;

        Ogre::Vector3 position = Ogre::Vector3(pos.x - (chunkX * slotSize), pos.y, pos.z - (chunkY * slotSize)) + origin.position();

        //Check the negative borrowing overflow.
        if(position.x < 0){
            chunkX -= 1;
            position.x = slotSize + position.x;
        }
        if(position.z < 0){
            chunkY -= 1;
            position.z = slotSize + position.z;
        }

        //Now make it relative to the origin.
        _chunkX = chunkX + origin.chunkX();
        _chunkY = chunkY + origin.chunkY();
        //The position is more complicated though, incase it overflows into another slot.
        //Get the size of the difference between the two slot positions.

        //This will tell us if the difference is worth overflowing for.
        int diffX = position.x / slotSize;
        int diffY = position.z / slotSize;

        position.x = position.x - (diffX * slotSize);
        position.z = position.z - (diffY * slotSize);

        //Add the difference
        _chunkX += diffX;
        _chunkY += diffY;

        _position = position;
    }

    bool SlotPosition::operator==(const SlotPosition &pos) const{
        if(pos.chunkX() == _chunkX && pos.chunkY() == _chunkY && pos.position() == _position) return true;
        else return false;
    }

    SlotPosition& SlotPosition::operator=(const SlotPosition &pos){
        _chunkX = pos.chunkX();
        _chunkY = pos.chunkY();
        _position = pos.position();

        return *this;
    }

    SlotPosition SlotPosition::operator+(const SlotPosition &pos){
        int retX = _chunkX + pos.chunkX();
        int retY = _chunkY + pos.chunkY();
        Ogre::Vector3 retPos = _position + pos._position;

        //The positions should fit within a single slot size.
        //The two positions can add to overflow a single slot, so this needs to be accounted for.
        //This means I should be able to do a single check if it's greater than the slot size.
        //If it is then take a slot size away from it and add 1 to the respective number.
        //Negative numbers don't need to be considered as they don't exist in this coordinate system.
        int slotSize = SystemSettings::getWorldSlotSize();
        if(retPos.x > slotSize){
            retX += 1;
            retPos.x -= slotSize;
        }
        if(retPos.z > slotSize){
            retY += 1;
            retPos.z -= slotSize;
        }

        return SlotPosition(retX, retY, retPos);
    }

    SlotPosition SlotPosition::operator-(const SlotPosition &pos){
        int retX = _chunkX - pos.chunkX();
        int retY = _chunkY - pos.chunkY();
        Ogre::Vector3 retPos = _position - pos._position;

        int slotSize = SystemSettings::getWorldSlotSize();
        if(retPos.x < 0){
            retX -= 1;
            retPos.x += slotSize;
        }
        if(retPos.z < 0){
            retY -= 1;
            retPos.z += slotSize;
        }

        return SlotPosition(retX, retY, retPos);
    }

    SlotPosition SlotPosition::operator+(const Ogre::Vector3 &ammount){
        int retX = _chunkX;
        int retY = _chunkY;
        Ogre::Vector3 retPos = _position + ammount;

        //Check if there's any difference in the positions.
        int slotSize = SystemSettings::getWorldSlotSize();
        int chunkXRemainder = retPos.x / slotSize;
        int chunkYRemainder = retPos.z / slotSize;
        if(chunkXRemainder != 0){
            retX += chunkXRemainder;
            retPos.x -= chunkXRemainder*slotSize;
        }
        if(chunkYRemainder != 0){
            retY += chunkYRemainder;
            retPos.z -= chunkYRemainder*slotSize;
        }

        //There is still a chance that the ammount passed in didn't trip the previous set of ifs.
        //(-60, 0, -50)
        //Those values don't divide if the slot size was 100, but they still need to be broken down.
        //If retPos is still negative by this point then the aforementioned case must be true.
        if(retPos.x < 0){
            retX -= 1;
            retPos.x = slotSize + retPos.x;
        }
        if(retPos.z < 0){
            retY -= 1;
            retPos.z = slotSize + retPos.z;
        }

        return SlotPosition(retX, retY, retPos);
    }

    SlotPosition SlotPosition::operator-(const Ogre::Vector3 &ammount){
        int retX = _chunkX;
        int retY = _chunkY;
        Ogre::Vector3 retPos = _position - ammount;

        int slotSize = SystemSettings::getWorldSlotSize();
        int chunkXRemainder = retPos.x / slotSize;
        int chunkYRemainder = retPos.z / slotSize;
        if(chunkXRemainder != 0){
            retX += chunkXRemainder;
            retPos.x -= chunkXRemainder*slotSize;
        }
        if(chunkYRemainder != 0){
            retY += chunkYRemainder;
            retPos.z -= chunkYRemainder*slotSize;
        }

        if(retPos.x < 0){
            retX -= 1;
            retPos.x = slotSize + retPos.x;
        }
        if(retPos.z < 0){
            retY -= 1;
            retPos.z = slotSize + retPos.z;
        }

        return SlotPosition(retX, retY, retPos);
    }

    Ogre::Vector3 SlotPosition::toOgre() const{
        const SlotPosition& origin = WorldSingleton::getOrigin();
        int slotSize = SystemSettings::getWorldSlotSize();

        int offsetX = _chunkX - origin.chunkX();
        int offsetY = _chunkY - origin.chunkY();
        Ogre::Vector3 posDiff = _position - origin.position();
        Ogre::Vector3 dest = Ogre::Vector3(offsetX * slotSize, 0, offsetY * slotSize) + posDiff;

        return dest;
    }

    std::ostream& operator << (std::ostream& o, const SlotPosition &coord){
        o << "SlotPosition(" << coord.chunkX() << ", " << coord.chunkY() << ", " << coord.position() << ")";
        return o;
    }
}
