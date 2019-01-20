#include "ChunkFactory.h"

#include "World/Slot/ChunkCoordinate.h"
#include "World/Slot/Recipe/OgreMeshRecipeData.h"
#include "World/Slot/Recipe/RecipeData.h"

#include "Logger/Log.h"

#include "Chunk.h"

#include "OgreRoot.h"

namespace AV{
    ChunkFactory::ChunkFactory(){
        mSceneManager = Ogre::Root::getSingletonPtr()->getSceneManager("Scene Manager");

        mStaticShapeNode = mSceneManager->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_STATIC);
    }

    ChunkFactory::~ChunkFactory(){

    }

    bool ChunkFactory::deconstructChunk(Chunk* chunk){
        if(!chunk) return false;

        Ogre::SceneNode *node = chunk->getStaticMeshNode();

        mSceneManager->destroySceneNode(node);
    }

    Chunk* ChunkFactory::constructChunk(const RecipeData &recipe, bool position){
        if(!recipe.recipeReady){
            AV_WARN("The chunk factory can't construct an unfinished chunk {}", recipe.coord);
            return 0;
        }

        AV_INFO("Constructing chunk {}", recipe.coord);

        Ogre::SceneNode *parentNode = mStaticShapeNode->createChildSceneNode(Ogre::SCENE_STATIC);
        for(const OgreMeshRecipeData &i : *recipe.ogreMeshData){
            Ogre::SceneNode *node = parentNode->createChildSceneNode(Ogre::SCENE_STATIC);

            Ogre::Item *item = mSceneManager->createItem(i.meshName+".mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_STATIC);
            node->attachObject((Ogre::MovableObject*)item);

            node->setPosition(i.pos);
            node->setScale(i.scale);
        }
        parentNode->setVisible(false);
        //parentNode->setPosition

        Chunk *c = new Chunk(recipe.coord, parentNode);

        return c;
    }

};
