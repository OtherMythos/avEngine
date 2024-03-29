#include "ChunkRadiusLoader.h"
#include "System/SystemSetup/SystemSettings.h"
#include "World/Slot/ChunkCoordinate.h"

#include "World/Slot/SlotManager.h"
#include "World/WorldSingleton.h"

#include "Logger/Log.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"
#include "Event/Events/ChunkEvent.h"

namespace AV{
    ChunkRadiusLoader::ChunkRadiusLoader(std::shared_ptr<SlotManager> slotManager)
    : mSlotManager(slotManager){
        initialise();
    }

    ChunkRadiusLoader::ChunkRadiusLoader(){

    }

    ChunkRadiusLoader::~ChunkRadiusLoader(){
        AV_INFO("Shutting down the ChunkRadiusLoader.");
        EventDispatcher::unsubscribe(EventType::World, this);
    }

    void ChunkRadiusLoader::initialise(){
        EventDispatcher::subscribe(EventType::World, AV_BIND(ChunkRadiusLoader::worldEventReceiver));
    }

    bool ChunkRadiusLoader::chunkLoadedInCurrentMap(int chunkX, int chunkY) const{
        LoadedChunkData d(chunkX, chunkY);

        if(mLoadedChunks.find(d) != mLoadedChunks.end()) return true;

        return false;
    }

    void ChunkRadiusLoader::_unloadEverything(){
        if(mLoadedChunks.size() <= 0) return;

        for(const LoadedChunkData& d : mLoadedChunks){
            //I destroy them individually to avoid issues with pending chunks.
            //It would be more efficient to have a blanket destroy of them all in the SlotManager.
            //However there is no guarantee that all chunks in the slot manager are being managed by the radius loader.
            //In this case I would want to destroy only the ones being managed.
            _unloadChunk(d);
        }
        mLoadedChunks.clear();
    }

    void ChunkRadiusLoader::_updatePlayer(const SlotPosition &playerPos){
        int radius = WorldSingleton::getPlayerLoadRadius();
        if(radius <= 0) {
            //If the value is 0 we can just make sure everything is unloaded and do nothing.
            _unloadEverything();
            return;
        }

        //TODO make this immune to floating point precision issues by factoring in the origin.
        int chunkSize = SystemSettings::getWorldSlotSize();
        //To start find the centre position of the circle
        Ogre::Real circleX = (playerPos.chunkX() * chunkSize) + playerPos.position().x;
        Ogre::Real circleY = (playerPos.chunkY() * chunkSize) + playerPos.position().z;

        //The coordinates of the circle's rectangle
        Ogre::Real startX = circleX - radius;
        Ogre::Real startY = circleY - radius;
        Ogre::Real endX = circleX + radius;
        Ogre::Real endY = circleY + radius;

        //Find the actual chunk coordinates that lie within the circle's rectangle
        int startXTile = floor(startX / chunkSize);
        int startYTile = floor(startY / chunkSize);
        int endXTile = ceil(endX / chunkSize);
        int endYTile = ceil(endY / chunkSize);

        auto z = mLoadedChunks.begin();
        while(z != mLoadedChunks.end()){
            if(!_checkRectCircleCollision((*z).first, (*z).second, chunkSize, radius, circleX, circleY)){
                //If the chunk isn't within the bounds of the circle then it's stale, so remove it.
                _unloadChunk((*z));
                z = mLoadedChunks.erase(z);
            }else z++;
        }

        for (int y = startYTile; y < endYTile; y++) {
            for (int x = startXTile; x < endXTile; x++) {
                //Go through these chunks to determine what to load.
                if(_checkRectCircleCollision(x, y, chunkSize, radius, circleX, circleY)){
                    LoadedChunkData chunk(x, y);
                    if(mLoadedChunks.find(chunk) == mLoadedChunks.end()){
                        //The chunk is not in the loaded list, so it's safe to load it.
                        mLoadedChunks.insert(chunk);
                        _loadChunk(chunk);
                    }
                }
            }
        }
    }

    bool ChunkRadiusLoader::worldEventReceiver(const Event &e){
        const WorldEvent& event = (WorldEvent&)e;
        if(event.eventId() == EventId::WorldMapChange){
            const WorldEventMapChange& wEvent = (WorldEventMapChange&)event;
            _updateCurrentMap(wEvent.oldMapName, wEvent.newMapName);
        }else if(event.eventId() == EventId::WorldOriginChange){
            _updatePlayer(WorldSingleton::getPlayerPosition());
        }else if(event.eventId() == EventId::WorldPlayerRadiusChange){
            _updatePlayer(WorldSingleton::getPlayerPosition());
        }else if(event.eventId() == EventId::WorldPlayerPositionChange){
            _updatePlayer(WorldSingleton::getPlayerPosition());
        }
        return true;
    }

    void ChunkRadiusLoader::_updateCurrentMap(const Ogre::String& oldMap, const Ogre::String& newMap){
        AV_INFO("Shifting current map.");

        //The procedure to shift the map involves:
        //Clear all the currently loaded chunks.
        //I don't need to bother rechecking because the chunks'll be the same (unless the position has changed.)
        //I don't check position change here.

        //OPTIMISATION later when player teleportation is a thing this can be simplified to check the player position as well.
        //I'd want to avoid this work being done multiple times, as it's likely the player will move to a different position as well as switching maps.
        for(const LoadedChunkData &c : mLoadedChunks){
            mSlotManager->destroyChunk(ChunkCoordinate(c.first, c.second, oldMap));
        }

        for(const LoadedChunkData &c : mLoadedChunks){
            mSlotManager->activateChunk(ChunkCoordinate(c.first, c.second, newMap));
        }
    }

    void ChunkRadiusLoader::_loadChunk(const LoadedChunkData &chunk){
        AV_INFO("Load {} {}", chunk.first, chunk.second);
        mSlotManager->activateChunk(ChunkCoordinate(chunk.first, chunk.second, WorldSingleton::getCurrentMap()));

        ChunkEventChunkEntered e;
        e.chunkX = chunk.first;
        e.chunkY = chunk.second;
        EventDispatcher::transmitEvent(EventType::Chunk, e);
    }

    void ChunkRadiusLoader::_unloadChunk(const LoadedChunkData &chunk){
        AV_INFO("unload {} {}", chunk.first, chunk.second);
        mSlotManager->destroyChunk(ChunkCoordinate(chunk.first, chunk.second, WorldSingleton::getCurrentMap()));

        ChunkEventChunkLeft e;
        e.chunkX = chunk.first;
        e.chunkY = chunk.second;
        EventDispatcher::transmitEvent(EventType::Chunk, e);
    }

    bool ChunkRadiusLoader::_checkRectCircleCollision(int tileX, int tileY, int rectSize, int radius, int circleX, int circleY) const{
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
}
