#include "SlotPosition.h"

#include "World/WorldSingleton.h"
#include "System/SystemSetup/SystemSettings.h"

#include "OgreString.h"
#include "OgreStringConverter.h"
#include <regex>

namespace AV{

    const SlotPosition SlotPosition::ZERO;

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
            else if(_position.x < 0) _position.x = 0;
            if(_position.z > slotSize) _position.z = slotSize;
            else if(_position.z < 0) _position.z = 0;
    }

    SlotPosition::SlotPosition(int chunkX, int chunkY)
    : _chunkX(chunkX),
    _chunkY(chunkY),
    _position(Ogre::Vector3::ZERO){

    }

    SlotPosition::SlotPosition(const std::string& s){
        static const std::regex floatRegex("^-?\\d+ -?\\d+ \\d+\\.?\\d* \\d+\\.?\\d* \\d+\\.?\\d*$");
        if(!std::regex_match(s, floatRegex)){
            *this = SlotPosition();
            return;
        }

        Ogre::StringVector vec = Ogre::StringUtil::split(s);
        int slotX = Ogre::StringConverter::parseInt(vec[0]);
        int slotY = Ogre::StringConverter::parseInt(vec[1]);
        Ogre::Real posX = Ogre::StringConverter::parseReal(vec[2]);
        Ogre::Real posY = Ogre::StringConverter::parseReal(vec[3]);
        Ogre::Real posZ = Ogre::StringConverter::parseReal(vec[4]);

        *this = SlotPosition(slotX, slotY, Ogre::Vector3(posX, posY, posZ));
    }

    SlotPosition::SlotPosition(const SlotPosition &pos)
    :_chunkX(pos.chunkX()),
    _chunkY(pos.chunkY()),
    _position(pos.position()){

    }

    SlotPosition::SlotPosition(const btVector3 &pos)
        : SlotPosition(Ogre::Vector3(pos.x(), pos.y(), pos.z())){

        //SlotPosition::SlotPosition(ogrePos);
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

    void SlotPosition::setX(Ogre::Real x){
        int slotSize = SystemSettings::getWorldSlotSize();
        if(x > slotSize) _position.x = slotSize;
        else if(x < 0) _position.x = 0;
        else _position.x = x;
    }
    void SlotPosition::setZ(Ogre::Real z){
        int slotSize = SystemSettings::getWorldSlotSize();
        if(z > slotSize) _position.z = slotSize;
        else if(z < 0) _position.z = 0;
        else _position.z = z;
    }

    bool SlotPosition::operator==(const SlotPosition &pos) const{
        if(pos.chunkX() == _chunkX && pos.chunkY() == _chunkY && pos.position() == _position) return true;
        else return false;
    }

    bool SlotPosition::operator<(const SlotPosition &pos) const{
        if(pos.chunkX() < _chunkX && pos.chunkY() < _chunkY && pos.position() < _position) return true;
        else return false;
    }

    SlotPosition& SlotPosition::operator=(const SlotPosition &pos){
        _chunkX = pos.chunkX();
        _chunkY = pos.chunkY();
        _position = pos.position();

        return *this;
    }

    SlotPosition SlotPosition::operator+(const SlotPosition &pos) const{
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

    void SlotPosition::moveTowards(const SlotPosition &destination, Ogre::Real interval){
        const SlotPosition delta(destination - *this);

        Ogre::Vector3 pos = delta.toOgreAbsolute();
        Ogre::Real magnitude = pos.length();
        if(magnitude <= interval || magnitude == 0.0f){
            *this = destination;
            return;
        }

        *this += (pos / magnitude * interval);
    }

    SlotPosition SlotPosition::operator-(const SlotPosition &pos) const{
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

    void SlotPosition::_additionOperation(const Ogre::Vector3& amount, SlotPosition& pos) const{
        int retX = _chunkX;
        int retY = _chunkY;
        Ogre::Vector3 retPos = _position + amount;

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

        pos = SlotPosition(retX, retY, retPos);
    }

    SlotPosition SlotPosition::operator+(const Ogre::Vector3 &amount) const{
        SlotPosition pos;

        _additionOperation(amount, pos);
        return pos;
    }

    SlotPosition SlotPosition::operator+(const btVector3 &amount) const{
        SlotPosition pos;

        Ogre::Vector3 a(amount.x(), amount.y(), amount.z());
        _additionOperation(a, pos);
        return pos;
    }

    void SlotPosition::operator+=(const Ogre::Vector3& vec){
        *this = (*this + vec);
    }

    void SlotPosition::operator-=(const Ogre::Vector3& vec){
        *this = (*this - vec);
    }

    void SlotPosition::operator+=(const btVector3& vec){
        *this = (*this + Ogre::Vector3(vec.x(), vec.y(), vec.z()));
    }

    void SlotPosition::operator-=(const btVector3& vec){
        *this = (*this - Ogre::Vector3(vec.x(), vec.y(), vec.z()));
    }

    //There seems to be a small difference between these operations.
    //If the chunk size is set to 100, you can set a slot position with position 100 through the constructor.
    //However there are some functions here which will flip and increment the chunk coordinate if the position is exactly equal to 100.
    //That's probably the intended result, as including 0 there are 100 values. 1 in chunk x would mean 100.
    //So having 0, 0, (100, 0, 0) would make no sense as that's the same as 1, 0, (0, 0, 0)
    //I'm not sure how much of a problem this will pose.
    //I'm not sure how to deal with it. I could cap the max position at slotPosition -1, but I can see inaccuracies happening there.
    //It would most likely manifest itself if continually adding ogre vectors as there might be some juttering.
    //Just bear it in mind.
    void SlotPosition::operator+=(const SlotPosition &pos){
        int retX = _chunkX + pos.chunkX();
        int retY = _chunkY + pos.chunkY();
        Ogre::Vector3 retPos = _position + pos._position;

        int slotSize = SystemSettings::getWorldSlotSize();
        if(retPos.x > slotSize){
            retX += 1;
            retPos.x -= slotSize;
        }
        if(retPos.z > slotSize){
            retY += 1;
            retPos.z -= slotSize;
        }


        _chunkX = retX;
        _chunkY = retY;
        _position = retPos;
    }

    void SlotPosition::operator-=(const SlotPosition &pos){
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

        _chunkX = retX;
        _chunkY = retY;
        _position = retPos;
    }

    void SlotPosition::_minusOperation(const Ogre::Vector3& amount, SlotPosition& pos) const{
        int retX = _chunkX;
        int retY = _chunkY;
        Ogre::Vector3 retPos = _position - amount;

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

        pos = SlotPosition(retX, retY, retPos);
    }

    SlotPosition SlotPosition::operator-(const btVector3 &amount) const{
        SlotPosition pos;

        Ogre::Vector3 a(amount.x(), amount.y(), amount.z());
        _minusOperation(a, pos);
        return pos;
    }

    SlotPosition SlotPosition::operator-(const Ogre::Vector3 &amount) const{
        SlotPosition pos;

        _minusOperation(amount, pos);
        return pos;
    }

    Ogre::Vector3 SlotPosition::_ogreToOrigin(const SlotPosition& origin) const{
        int slotSize = SystemSettings::getWorldSlotSize();

        int offsetX = _chunkX - origin.chunkX();
        int offsetY = _chunkY - origin.chunkY();
        Ogre::Vector3 posDiff = _position - origin.position();
        Ogre::Vector3 dest = Ogre::Vector3(offsetX * slotSize, 0, offsetY * slotSize) + posDiff;

        return dest;
    }

    Ogre::Real SlotPosition::getDistanceBetween(const SlotPosition& pos) const{
        const Ogre::Vector3 a(toOgre());
        const Ogre::Vector3 b(pos.toOgre());
        return a.distance(b);
    }

    Ogre::Vector3 SlotPosition::toOgre() const{
        return _ogreToOrigin(WorldSingleton::getOrigin());
    }

    Ogre::Vector3 SlotPosition::toOgreAbsolute() const{
        return _ogreToOrigin(SlotPosition::ZERO);
    }

    Ogre::Vector3 SlotPosition::toOgreWithOrigin(const SlotPosition& origin) const{
        return _ogreToOrigin(origin);
    }

    btVector3 SlotPosition::toBullet() const{
        Ogre::Vector3 dest = toOgre();

        return btVector3(dest.x, dest.y, dest.z);
    }

    btVector3 SlotPosition::toBulletWithOrigin(const SlotPosition& origin) const{
        Ogre::Vector3 dest = toOgreWithOrigin(origin);

        return btVector3(dest.x, dest.y, dest.z);
    }

    std::ostream& operator << (std::ostream& o, const SlotPosition &coord){
        o << "SlotPosition(" << coord.chunkX() << ", " << coord.chunkY() << ", " << coord.position() << ")";
        return o;
    }

    void SlotPosition::printLeanStream(std::ostream& o) const{
        o << _chunkX << ", " << _chunkY << ", (" << _position.x << ", " << _position.y << ", " << _position.z << ")";
    }
}
