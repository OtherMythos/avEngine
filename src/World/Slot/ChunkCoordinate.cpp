#include "ChunkCoordinate.h"

namespace AV{
    const ChunkCoordinate ChunkCoordinate::DEFAULT;

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
        //This probably isn't the best way to do that.
        //TODO clean that up.
        std::string xVal = std::to_string(abs(_chunkX));
        std::string yVal = std::to_string(abs(_chunkY));

        std::string xString = std::string(4 - xVal.length(), '0') + xVal;
        std::string yString = std::string(4 - yVal.length(), '0') + yVal;
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
