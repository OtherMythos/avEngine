#pragma once

#include "OgreString.h"

namespace AV{
    /**
    Represents a chunk in the world.
    Contains an x and y chunk position as well as a map name string.

    @remarks
    This class does not provide direct access to the chunk x and y values, instead they should be accessed via the getters.
    This is to make sure that the values contained always fit within sensible bounds.
    */
    class ChunkCoordinate{
    private:
        int _chunkX, _chunkY;
        Ogre::String _mapName;

    public:
        ChunkCoordinate();
        ChunkCoordinate(int chunkX, int chunkY, const Ogre::String &mapName);
        ChunkCoordinate(const ChunkCoordinate &coord);

        int chunkX() const { return _chunkX; }
        int chunkY() const { return _chunkY; }
        const Ogre::String& mapName() const { return _mapName; }

        bool operator==(const ChunkCoordinate &pos) const;
        ChunkCoordinate& operator=(const ChunkCoordinate &pos);
        friend std::ostream& operator<<(std::ostream& o, const ChunkCoordinate &coord);

        /**
        Get the path to this chunk coordinate in the maps directory.

        @return
        A string representing the path to that directory.
        For instance if the chunk coordinate described the chunk (0, 0, map), that would convert to: map/00000000
        */
        std::string getFilePath() const;

        /**
        Get a string representing the coordinates of this chunk.
        For instance for chunk (0, 0) it would return 00000000.
        Negative numbers are also taken into account (-100, -200), -0100-0200.
        At the moment these chunks are described to four digits.
        */
        std::string getCoordsString() const;

        /**
        Get a string for use by the terrain to create a resource group.
        For the coordinate (10, 20, Map) it would generate TerrainMap00100020
        */
        std::string getTerrainGroupName() const;
    };
}
