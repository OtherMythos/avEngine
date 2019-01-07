#include "ChunkRadiusChecks.h"

#include <stdlib.h>
#include "World/WorldSingleton.h"
#include "System/SystemSetup/SystemSettings.h"

namespace AV {
    bool ChunkRadiusChecks::_checkRectCircleCollision(int tileX, int tileY, int rectSize, int radius, int circleX, int circleY){
        /*int distX = abs(circleX - (tileX * rectSize)-rectSize/2);
        int distY = abs(circleY - (tileY * rectSize)-rectSize/2);

        if(distX > (rectSize / 2 + radius)) return false;
        if(distY > (rectSize / 2 + radius)) return false;

        if(distX <= (rectSize/2)) return true;
        if(distY <= (rectSize/2)) return true;

        int dx = distX - rectSize / 2;
        int dy = distY - rectSize / 2;

        return (dx*dx+dy*dy<=(radius*radius));*/

        int distX = abs(circleX - (tileX)-rectSize/2);
        int distY = abs(circleY - (tileY)-rectSize/2);

        if(distX > (rectSize / 2 + radius)) return false;
        if(distY > (rectSize / 2 + radius)) return false;

        if(distX <= (rectSize/2)) return true;
        if(distY <= (rectSize/2)) return true;

        int dx = distX - rectSize / 2;
        int dy = distY - rectSize / 2;

        return (dx*dx+dy*dy<=(radius*radius));
    }

    bool ChunkRadiusChecks::_isChunkWithinRadius(int chunkX, int chunkY, const SlotPosition& pos, int radius){
        int slotSize = SystemSettings::getWorldSlotSize();

        for(int y = 0; y < 2; y++){
            for(int x = 0; x < 2; x++){
                int x1 = pos.chunkX() * slotSize;
                int x2 = (chunkX + x) * slotSize;
                int y1 = pos.chunkY() * slotSize;
                int y2 = (chunkY + y) * slotSize;
                double distance = sqrt(pow(x2-x1, 2) + pow(y2-y1, 2));

                if(distance > radius){
                    return false;
                }
            }
        }
        return true;
    }

    /*
     TODO At some point this should have two circles around the player for loading and unloading.
     The distance between them should be different to prevent the player trying to skip back and forth between the load boundary to try and break things.
     There should just be a function named something like isChunkViable() and it'll check based on that.
     */
    bool ChunkRadiusChecks::isChunkWithinOrigin(int chunkX, int chunkY){
        const int possibleDistance = 100;
        //Storing some of these values as slot positions allows me to do these calculations taking the origin into account.
        //This isn't of critical importance, but it should help prevent bigger numbers appearing in here.
        SlotPosition cPos(chunkX, chunkY);
        Ogre::Vector3 chunkPos = cPos.toOgre();
        Ogre::Vector3 playerPos = WorldSingleton::getOrigin().toOgre();

        return _checkRectCircleCollision(chunkPos.x, chunkPos.z, SystemSettings::getWorldSlotSize(), possibleDistance, playerPos.x, playerPos.z);
    }

    bool ChunkRadiusChecks::isChunkWithinPlayerPos(int chunkX, int chunkY){
        const int possibleDistance = 100;
        SlotPosition cPos(chunkX, chunkY);
        Ogre::Vector3 chunkPos = cPos.toOgre();
        Ogre::Vector3 playerPos = WorldSingleton::getPlayerPosition().toOgre();

        return _checkRectCircleCollision(chunkPos.x, chunkPos.z, SystemSettings::getWorldSlotSize(), possibleDistance, playerPos.x, playerPos.z);
    }
}
