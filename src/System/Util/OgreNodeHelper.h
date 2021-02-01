#pragma once

#include "OgreSceneManager.h"
#include "OgreSceneNode.h"

namespace AV{
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
            if(it.hasMoreElements()){
                Ogre::MovableObject* obj = it.getNext();
                node->getCreator()->destroyMovableObject(obj);
            }
            //At the moment a mesh object should only have the one mesh attached to it, so by this point there should be no more.
            assert(!it.hasMoreElements());
        }

        /**
        Destroy the children of a node only. This does not include the provided node or any items attached to it.
        */
        static void recursiveDestroyNode(Ogre::SceneNode* node){
            _recursiveDestroyMovableObjects(node);
            node->removeAndDestroyAllChildren();
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
