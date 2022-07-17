#include "AvSceneDataInserter.h"

#include <OgreSceneNode.h>
#include <OgreSceneManager.h>

namespace AV{

    AVSceneDataInserter::AVSceneDataInserter(Ogre::SceneManager* sceneManager)
        : mSceneManager(sceneManager) {

    }

    AVSceneDataInserter::~AVSceneDataInserter(){

    }

    void AVSceneDataInserter::insertSceneData(ParsedSceneFile* data, Ogre::SceneNode* node){
        size_t startIdx = 0;
        size_t end = _insertSceneData(startIdx, data, node);
        assert(end == data->objects.size());
    }

    size_t AVSceneDataInserter::_insertSceneData(size_t index, ParsedSceneFile* data, Ogre::SceneNode* parent){
        Ogre::SceneNode* previousNode = 0;

        size_t current = index;
        for(; current < data->objects.size(); current+=0){
            const SceneObjectEntry& entry = data->objects[current];
            if(entry.type == SceneObjectType::Child){
                assert(previousNode);
                current = _insertSceneData(current + 1, data, previousNode);
                continue;
            }else if(entry.type == SceneObjectType::Term){
                return current + 1;
            }else{
                previousNode = _createObject(entry, parent);
                current++;
            }
        }

        return current;
    }

    Ogre::SceneNode* AVSceneDataInserter::_createObject(const SceneObjectEntry& e, Ogre::SceneNode* parent){
        Ogre::SceneNode* newNode = parent->createChildSceneNode();

        switch(e.type){
            case SceneObjectType::Empty:{
                break;
            }
            case SceneObjectType::Mesh:{
                Ogre::Item *item = mSceneManager->createItem("cube", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_DYNAMIC);
                newNode->attachObject((Ogre::MovableObject*)item);
                break;
            }
            default:{
                assert(false);
            }
        }

        newNode->setPosition(e.pos);
        newNode->setScale(e.scale);
        newNode->setOrientation(e.orientation);

        return newNode;
    }
}
