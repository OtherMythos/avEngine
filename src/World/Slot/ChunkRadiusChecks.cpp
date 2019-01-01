#include "ChunkRadiusChecks.h"

#include <stdlib.h>

namespace AV {
    bool ChunkRadiusChecks::_checkRectCircleCollision(int tileX, int tileY, int rectSize, int radius, int circleX, int circleY){
        int distX = abs(circleX - (tileX * rectSize)-rectSize/2);
        int distY = abs(circleY - (tileY * rectSize)-rectSize/2);
        
        if(distX > (rectSize / 2 + radius)) return false;
        if(distY > (rectSize / 2 + radius)) return false;
        
        if(distX <= (rectSize/2)) return true;
        if(distY <= (rectSize/2)) return true;
        
        int dx = distX - rectSize / 2;
        int dy = distY - rectSize / 2;
        
        return (dx*dx+dy*dy<=(radius*radius));
    }
    
    bool ChunkRadiusChecks::isChunkWithinOrigin(int chunkX, int chunkY){
        return false;
        const int possibleDistance = 4000;
        
        bool possible = true;
        for(int y = 0; y < 2; y++){
            for(int x = 0; x < 2; x++){
//                int x1 = origin.chunkX * CHUNK_SIZE_WORLD;
//                int x2 = (chunk.chunkX + x) * CHUNK_SIZE_WORLD;
//                int y1 = origin.chunkY * CHUNK_SIZE_WORLD;
//                int y2 = (chunk.chunkY + y) * CHUNK_SIZE_WORLD;
//                double distance = sqrt(pow(x2-x1, 2) + pow(y2-y1, 2));
//
//                if(distance > possibleDistance){
//                    possible = false;
//                    break;
//                }
            }
        }
        return possible;
    }
}
