#include "AvSceneDataInserter.h"

#include <OgreSceneNode.h>
#include <OgreSceneManager.h>

#include "World/Slot/Recipe/AvScene/AvSceneFileParser.h"
#include "System/BaseSingleton.h"
#include "Animation/AnimationManager.h"

namespace AV{

    AVSceneDataInserter::AVSceneDataInserter(Ogre::SceneManager* sceneManager)
        : mSceneManager(sceneManager) {

    }

    AVSceneDataInserter::~AVSceneDataInserter(){

    }

    //TODO in theory I could have a different data type which avoids the animation entries.
    AnimationInfoBlockPtr AVSceneDataInserter::insertSceneDataGetAnimInfo(ParsedSceneFile* data, Ogre::SceneNode* node){
        memset(&animInfo, 0, sizeof(animInfo));
        animHash = 0;

        size_t startIdx = 0;
        size_t createdObjectCount = 0;
        size_t end = _insertSceneData(startIdx, createdObjectCount, data, node);
        assert(end == data->objects.size());

        if(animHash == 0){
            return 0;
        }

        assert(animHighestIdx > 0);
        AnimationInfoBlockPtr ptr = BaseSingleton::getAnimationManager()->createAnimationInfoBlock(animInfo, animHighestIdx, animHash);

        return ptr;
    }

    void AVSceneDataInserter::insertSceneData(ParsedSceneFile* data, Ogre::SceneNode* node){
        size_t startIdx = 0;
        size_t createdObjectCount = 0;
        size_t end = _insertSceneData(startIdx, createdObjectCount, data, node);
        assert(end == data->objects.size());
    }

    size_t AVSceneDataInserter::_insertSceneData(size_t index, size_t& createdObjectCount, ParsedSceneFile* data, Ogre::SceneNode* parent){
        Ogre::SceneNode* previousNode = 0;

        size_t current = index;
        for(; current < data->objects.size(); current+=0){
            const SceneObjectEntry& entry = data->objects[current];
            if(entry.type == SceneObjectType::Child){
                assert(previousNode);
                current = _insertSceneData(current + 1, createdObjectCount, data, previousNode);
                continue;
            }else if(entry.type == SceneObjectType::Term){
                return current + 1;
            }else{
                const SceneObjectData& sceneObjData = data->data[createdObjectCount];
                previousNode = _createObject(entry, sceneObjData, data->strings, parent);
                createdObjectCount++;
                current++;
            }
        }

        return current;
    }

    Ogre::SceneNode* AVSceneDataInserter::_createObject(const SceneObjectEntry& e, const SceneObjectData& d, const std::vector<Ogre::String>& strings, Ogre::SceneNode* parent){
        Ogre::SceneNode* newNode = parent->createChildSceneNode();

        newNode->setPosition(d.pos);
        newNode->setScale(d.scale);
        newNode->setOrientation(d.orientation);

        switch(e.type){
            case SceneObjectType::Empty:{
                break;
            }
            case SceneObjectType::Mesh:{
                const Ogre::String& meshName = strings[d.idx];
                Ogre::Item *item = mSceneManager->createItem(meshName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_DYNAMIC);
                newNode->attachObject((Ogre::MovableObject*)item);
                break;
            }
            case SceneObjectType::User0:{
                insertUserObject(0, e, d, strings, newNode);
                break;
            }
            case SceneObjectType::User1:{
                insertUserObject(1, e, d, strings, newNode);
                break;
            }
            case SceneObjectType::User2:{
                insertUserObject(2, e, d, strings, newNode);
                break;
            }
            case SceneObjectType::User3:{
                insertUserObject(3, e, d, strings, newNode);
                break;
            }
            case SceneObjectType::User4:{
                insertUserObject(4, e, d, strings, newNode);
                break;
            }
            case SceneObjectType::User5:{
                insertUserObject(5, e, d, strings, newNode);
                break;
            }
            case SceneObjectType::User6:{
                insertUserObject(6, e, d, strings, newNode);
                break;
            }
            default:{
                assert(false);
            }
        }

        if(d.animIdx != AVSceneFileParserInterface::NONE_ANIM_IDX){
            assert(d.animIdx < MAX_ANIMATION_INFO);
            animInfo[d.animIdx].sceneNode = newNode;
            animHash |= ANIM_INFO_SCENE_NODE << MAX_ANIMATION_INFO_BITS*d.animIdx;
            //This doesn't check for holes but just keeps track of the highest index.
            if(d.animIdx+1 > animHighestIdx){
                animHighestIdx = d.animIdx+1;
            }
        }

        return newNode;
    }

    bool AVSceneDataInserter::insertUserObject(AV::uint8 idx, const SceneObjectEntry& e, const SceneObjectData& d, const std::vector<Ogre::String>& strings, Ogre::SceneNode* parent){
        return true;
    }
}
