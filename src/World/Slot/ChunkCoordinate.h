#pragma once

#include "OgreString.h"

namespace AV{
    class ChunkCoordinate{
    private:
        int _chunkX, _chunkY;
        Ogre::String _mapName;

    public:
        ChunkCoordinate();
        ChunkCoordinate(int chunkX, int chunkY, const Ogre::String &mapName);

        int chunkX() const { return _chunkX; }
        int chunkY() const { return _chunkY; }
        const Ogre::String& mapName() const { return _mapName; }

        bool operator==(const ChunkCoordinate &pos) const;
        ChunkCoordinate& operator=(const ChunkCoordinate &pos);
        friend std::ostream& operator<<(std::ostream& o, const ChunkCoordinate &coord);
    };
}
