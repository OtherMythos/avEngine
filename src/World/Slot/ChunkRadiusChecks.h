#pragma once

namespace AV {
    class SlotPosition;

    /**
     A class to process checks on the chunk system.
     */
    class ChunkRadiusChecks{
    private:

        /**
         Check whether a tile lies within the bounds of a circle.
         This function is used internally to check if the player has entered or left a world chunk.

         @param tileX The x tile coordinate.
         @param tileY The y tile coordinate.
         @param rectSize The size of a tile.
         @param radius The radius of the check circle.
         @param circleX The x coordinate of the check circle.
         @param circleY The y coordinate of the check circle.
         */
        static bool _checkRectCircleCollision(int tileX, int tileY, int rectSize, int radius, int circleX, int circleY);

        static bool _isChunkWithinRadius(int chunkX, int chunkY, const SlotPosition& pos, int radius);

    public:
        /**
         Determine if the chunk coordinate provided fits within the distance from the origin.

         @return
         True if the chunk lies within the distance.
         */
        static bool isChunkWithinOrigin(int chunkX, int chunkY);

        static bool isChunkWithinPlayerPos(int chunkX, int chunkY);
    };
}
