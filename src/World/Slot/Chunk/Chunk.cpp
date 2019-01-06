#include "Chunk.h"

#include <Ogre.h>
#include "World/Slot/SlotPosition.h"

namespace AV{
    Chunk::Chunk(const std::string &map, int chunkX, int chunkY, Ogre::SceneNode *parentNode)
    : _map(map),
    _chunkX(chunkX),
    _chunkY(chunkY),
    _parentSceneNode(parentNode){
        _initialise();
    }

    Chunk::~Chunk(){
        Ogre::SceneNode *parent = (Ogre::SceneNode*)_parentSceneNode->getParent();
        parent->removeAndDestroyChild(_parentSceneNode);
    }

    void Chunk::_initialise(){
        //This is temporary until the item creation can be moved somewhere else.
        _sceneManager = Ogre::Root::getSingleton().getSceneManager("Scene Manager");

        //For now just pretend that no data is loaded from the disks about the chunks.
        //So create the meshes directly here.
        Ogre::Item *item = _sceneManager->createItem("ogrehead2.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_STATIC);
        _parentSceneNode->attachObject((Ogre::MovableObject*)item);
        _parentSceneNode->setVisible(false);

        _positionOgreMeshes();
    }

    bool Chunk::compare(const std::string &map, int chunkX, int chunkY){
        if(chunkX == _chunkX && chunkY == _chunkY && map == _map) return true;
        else return false;
    }

    void Chunk::activate(){
        if(_activated) return;

        _parentSceneNode->setVisible(true);


        _activated = true;
    }

    void Chunk::deactivate(){
        if(!_activated) return;

        _parentSceneNode->setVisible(false);

        _activated = false;
    }

    void Chunk::move(){
        if(!_activated) return;

        _positionOgreMeshes();
    }

    void Chunk::_positionOgreMeshes(){
        SlotPosition pos = SlotPosition(_chunkX, _chunkY);
        Ogre::Vector3 start = pos.toOgre();

        _parentSceneNode->setPosition(start);
    }
};
