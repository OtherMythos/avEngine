#include "ChunkCoordinate.h"

#include <fstream>

namespace AV{
    const ChunkCoordinate ChunkCoordinate::DEFAULT;

    const int ChunkCoordinate::MAX_CHUNK = 9999;
    const unsigned char ChunkCoordinate::CHUNK_DIGITS = 4;

    ChunkCoordinate::ChunkCoordinate()
        : _chunkX(0),
        _chunkY(0),
        _mapName(""){

    }

    ChunkCoordinate::ChunkCoordinate(int chunkX, int chunkY, const Ogre::String &mapName)
        : _chunkX(chunkX),
        _chunkY(chunkY),
        _mapName(mapName){

    }

    ChunkCoordinate::ChunkCoordinate(const ChunkCoordinate &coord){
        _chunkX = coord.chunkX();
        _chunkY = coord.chunkY();
        _mapName = coord.mapName();
    }

    bool ChunkCoordinate::operator==(const ChunkCoordinate &coord) const{
        if(coord.chunkX() == _chunkX && coord.chunkY() == _chunkY && coord.mapName() == _mapName) return true;
        else return false;
    }

    ChunkCoordinate& ChunkCoordinate::operator=(const ChunkCoordinate &coord){
        _chunkX = coord.chunkX();
        _chunkY = coord.chunkY();
        _mapName = coord.mapName();

        return *this;
    }

    std::ostream& operator << (std::ostream& o, const ChunkCoordinate &coord){
        o << "ChunkCoordinate(" << coord.mapName() << ", " << coord._chunkX << ", " << coord._chunkY << ")";
        return o;
    }

    std::string ChunkCoordinate::getCoordsString() const{

        if(_chunkX < -MAX_CHUNK ||_chunkY < -MAX_CHUNK || _chunkX > MAX_CHUNK || _chunkY > MAX_CHUNK) return "null";

        std::string xVal = std::to_string(abs(_chunkX));
        std::string yVal = std::to_string(abs(_chunkY));

        std::string xString = std::string(CHUNK_DIGITS - xVal.length(), '0') + xVal;
        std::string yString = std::string(CHUNK_DIGITS - yVal.length(), '0') + yVal;
        if(_chunkX < 0) xString = "-" + xString;
        if(_chunkY < 0) yString = "-" + yString;

        return xString + yString;
    }

    std::string ChunkCoordinate::getTerrainGroupName() const{
        return "Terra" + _mapName + getCoordsString();
    }

    std::string ChunkCoordinate::getFilePath() const{
        return _mapName + "/" + getCoordsString();
    }

};
