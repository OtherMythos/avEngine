#pragma once

#include "AvSceneFileParser.h"

#include <iostream>

#include "OgreItem.h"
#include "OgreSceneManager.h"

namespace AV{
    //Interface for constructing from an avScene file in the previewer.
    class SimpleSceneFileParserInterface : public AVSceneFileParserInterface{
    private:
        Ogre::SceneManager* mManager;
        Ogre::SceneNode* mParentNode;
        std::vector<Ogre::SceneNode*> mNodes;

        Ogre::SceneNode* _getNodeForId(int id){
            if(id < 0) return mParentNode;

            return mNodes[id];
        }
        int _pushNode(Ogre::SceneNode* node){
            int id = mNodes.size();
            mNodes.push_back(node);
            return id;
        }

    public:
        SimpleSceneFileParserInterface(Ogre::SceneManager* manager, Ogre::SceneNode* parentNode)
            : mManager(manager),
            mParentNode(parentNode){

        }

        void reachedEndForParent(int parent){ }

        void logError(const char* message){
            std::cerr << message << std::endl;
        }
        void log(const char* message){
            std::cout << message << std::endl;
        }

        int createEmpty(int parent, const Ogre::Vector3& pos, const Ogre::Vector3& scale, const Ogre::Quaternion& orientation){
            log("creating node");
            Ogre::SceneNode* node = _getNodeForId(parent);
            Ogre::SceneNode* newNode = node->createChildSceneNode();
            newNode->setPosition(pos);
            newNode->setScale(scale);
            newNode->setOrientation(orientation);

            return _pushNode(newNode);
        }
        int createMesh(int parent, const char* name, const char* mesh, const Ogre::Vector3& pos, const Ogre::Vector3& scale, const Ogre::Quaternion& orientation){
            Ogre::SceneNode* parentNode = _getNodeForId(parent);

            log("creating mesh");

            Ogre::SceneNode *node = parentNode->createChildSceneNode();
            Ogre::Item *item = mManager->createItem(mesh, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_DYNAMIC);
            node->attachObject((Ogre::MovableObject*)item);
            node->setPosition(pos);
            node->setScale(scale);
            node->setOrientation(orientation);

            return _pushNode(node);
        }
    };
}
