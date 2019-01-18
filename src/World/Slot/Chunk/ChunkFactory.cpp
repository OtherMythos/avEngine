#include "ChunkFactory.h"

#include "World/Slot/ChunkCoordinate.h"
#include "World/Slot/Recipe/OgreMeshRecipeData.h"
#include "World/Slot/Recipe/RecipeData.h"

#include "Chunk.h"

#include "OgreRoot.h"

namespace AV{
    ChunkFactory::ChunkFactory(){
        mSceneManager = Ogre::Root::getSingletonPtr()->getSceneManager("Scene Manager");

        mStaticShapeNode = mSceneManager->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_STATIC);
    }

    ChunkFactory::~ChunkFactory(){

    }

    Chunk* ChunkFactory::constructChunk(const RecipeData &recipe){
        Ogre::SceneNode *parentNode = mStaticShapeNode->createChildSceneNode(Ogre::SCENE_STATIC);
        parentNode->setVisible(false);
        for(const OgreMeshRecipeData &i : *recipe.ogreMeshData){
            Ogre::SceneNode *node = parentNode->createChildSceneNode(Ogre::SCENE_STATIC);

            Ogre::Item *item = mSceneManager->createItem(i.meshName+".mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_STATIC);
            node->attachObject((Ogre::MovableObject*)item);

            node->setPosition(i.pos);
            node->setScale(i.scale);
        }

        Chunk *c = new Chunk(recipe.coord, parentNode);

        return c;
    }

};
