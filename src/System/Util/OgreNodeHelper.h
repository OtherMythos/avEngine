#pragma once

#include "OgreSceneManager.h"
#include "OgreSceneNode.h"

namespace AV{

    /**
    Static functions to help alter Ogre scene nodes and items.
    */
    class OgreNodeHelper{
    public:
        OgreNodeHelper() = delete;
        ~OgreNodeHelper() = delete;

        /**
        Destroy a node and all its children, including the node itself.
        */
        static void destroyNodeAndChildren(Ogre::SceneNode* node){
            recursiveDestroyNode(node);
            //Destroy the parent node now.
            destroyMovableObject(node);
            node->getCreator()->destroySceneNode(node);
        }

        static void destroyMovableObject(Ogre::SceneNode* node){
            //When it comes time to destroy a mesh (movable object) a bit more work needs to be done, as you have to check it's actually there.
            Ogre::SceneNode::ObjectIterator it = node->getAttachedObjectIterator();
            while(it.hasMoreElements()){
                Ogre::MovableObject* obj = it.getNext();
                node->getCreator()->destroyMovableObject(obj);
                it = node->getAttachedObjectIterator();
            }
        }

        /**
        Destroy the children of a node only. This does not include the provided node or any items attached to it.
        */
        static void recursiveDestroyNode(Ogre::SceneNode* node){
            _recursiveDestroyMovableObjects(node);
            node->removeAndDestroyAllChildren();
        }

        /**
        Destroy the objects attached to a node. Do not destroy its children or their attached objects
        */
        static void recursiveDestroyAttachedObjects(Ogre::SceneNode* node){
            destroyMovableObject(node);
            node->detachAllObjects();
        }

    private:

        static void _recursiveDestroyMovableObjects(Ogre::SceneNode* node){
            auto it = node->getChildIterator();
            while(it.current() != it.end()){
                Ogre::SceneNode *n = (Ogre::SceneNode*)it.getNext();
                recursiveDestroyNode(n);
                destroyMovableObject(n);
            }
        }

    };
}
