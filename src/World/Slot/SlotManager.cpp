#include "SlotManager.h"

#include "World/WorldSingleton.h"

#include "Chunk/Chunk.h"
#include <Ogre.h>

namespace AV {
    SlotManager::SlotManager(){

    }

    SlotManager::~SlotManager(){

    }

    void SlotManager::initialise(){
      Ogre::SceneManager* sceneManager = Ogre::Root::getSingleton().getSceneManager("Scene Manager");
      _parentSlotNode = sceneManager->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_STATIC);
    }

    void SlotManager::updateChunks(const SlotPosition &playerPos){
        AV_INFO(playerPos.chunkX());
        AV_INFO(playerPos.position());
        std::vector<Chunk*>::iterator z = _activeChunks.begin();
        while(z != _activeChunks.end()){
            if(!ChunkRadiusChecks::isChunkWithinOrigin((*z)->getChunkX(), (*z)->getChunkY())){
                //The active chunk is no longer within the origin so should become inactive.
                AV_INFO("FOUND ONE");
                z++;
            }else z++;
        }
    }

    void SlotManager::setCurrentMap(const std::string &map){
        _currentMap = map;
    }

    bool SlotManager::loadChunk(const std::string &map, int chunkX, int chunkY){
        if(chunkX < 0 || chunkY < 0) return false;
        if(_checkIfChunkLoaded(map, chunkX, chunkY)) return false;

        Ogre::SceneNode *chunkNode = _parentSlotNode->createChildSceneNode(Ogre::SCENE_STATIC);
        Chunk* chunk = new Chunk(map, chunkX, chunkY, chunkNode);
        if(ChunkRadiusChecks::isChunkWithinOrigin(chunkX, chunkY)){
            if(map == _currentMap){
                _activeChunks.push_back(chunk);
                chunk->activate();
            }else{
                _loadedChunks.push_back(chunk);
            }
        }else{
            if(map == _currentMap){
                _loadedChunksCurrentMap.insert(chunk);
            }
            _loadedChunks.push_back(chunk);
        }

        return true;
    }

    bool SlotManager::unloadChunk(const std::string &map, int chunkX, int chunkY){
        std::vector<Chunk*>::iterator z = _activeChunks.begin();
        while(z != _activeChunks.end()){
            if((*z)->compare(map, chunkX, chunkY)){
                delete (*z);
                _activeChunks.erase(z);
                return true;
            }else z++;
        }

        std::vector<Chunk*>::iterator i = _loadedChunks.begin();
        while(i != _loadedChunks.end()){
            if((*i)->compare(map, chunkX, chunkY)){
                delete (*i);
                _loadedChunks.erase(i);
                return true;
            }else i++;
        }

        return false;
    }

    void SlotManager::setOrigin(const SlotPosition &pos){
        if(_getOrigin() == pos) return;

        WorldSingleton::_origin = pos;
    }

    const SlotPosition& SlotManager::_getOrigin(){
        return WorldSingleton::getOrigin();
    }

    bool SlotManager::_checkIfChunkLoaded(const std::string &map, int chunkX, int chunkY){
        for(Chunk *c : _activeChunks){
            if(c->compare(map, chunkX, chunkY))
                return true;
        }
        for(Chunk *c : _loadedChunks){
            if(c->compare(map, chunkX, chunkY))
                return true;
        }
        return false;
    }
}
