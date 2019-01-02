#pragma once

namespace AV {
    class ChunkRadiusChecks{
        static bool _checkRectCircleCollision(int tileX, int tileY, int rectSize, int radius, int circleX, int circleY);

    public:
        /**
         Determine if the chunk coordinate provided fits within the distance from the origin.

         @return
         True if the chunk lies within the distance.
         */
        static bool isChunkWithinOrigin(int chunkX, int chunkY);
    };
}
