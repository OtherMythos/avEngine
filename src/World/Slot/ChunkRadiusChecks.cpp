#include "ChunkRadiusChecks.h"

#include <stdlib.h>
#include "World/WorldSingleton.h"
#include "System/SystemSetup/SystemSettings.h"

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

    /*
     TODO At some point this should have two circles around the player for loading and unloading.
     The distance between them should be different to prevent the player trying to skip back and forth between the load boundary to try and break things.
     There should just be a function named something like isChunkViable() and it'll check based on that.
     */
    bool ChunkRadiusChecks::isChunkWithinOrigin(int chunkX, int chunkY){
        return false;
        const int possibleDistance = 4000;

        int slotSize = SystemSettings::getWorldSlotSize();
        const SlotPosition& origin = WorldSingleton::getOrigin();

        bool possible = true;
        for(int y = 0; y < 2; y++){
            for(int x = 0; x < 2; x++){
                int x1 = origin.chunkX * slotSize;
                int x2 = (chunkX + x) * slotSize;
                int y1 = origin.chunkY * slotSize;
                int y2 = (chunkY + y) * slotSize;
                double distance = sqrt(pow(x2-x1, 2) + pow(y2-y1, 2));

                if(distance > possibleDistance){
                    possible = false;
                    break;
                }
            }
        }
        return possible;
    }
}
