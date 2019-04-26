#include "ChunkFactory.h"

#include "World/Slot/ChunkCoordinate.h"
#include "World/Slot/SlotPosition.h"
#include "World/Slot/Recipe/OgreMeshRecipeData.h"
#include "World/Slot/Recipe/RecipeData.h"

#include "Logger/Log.h"

#include "Chunk.h"

#include "OgreRoot.h"
#include "Threading/JobDispatcher.h"
#include "Threading/Jobs/RecipeOgreMeshJob.h"

namespace AV{
    ChunkFactory::ChunkFactory(){

    }

    void ChunkFactory::initialise(){
        mSceneManager = Ogre::Root::getSingletonPtr()->getSceneManager("Scene Manager");

        mStaticShapeNode = mSceneManager->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_STATIC);
    }

    ChunkFactory::~ChunkFactory(){
        
    }
    
    void ChunkFactory::shutdown(){
        for(int i  = 0; i < RecipeData::MaxRecipies; i++){
            AV_INFO("Waiting for job {} in shutdown", mRunningRecipeJobs[i].id());
            JobDispatcher::endJob(mRunningRecipeJobs[i]);
        }
        
        if(mStaticShapeNode) mSceneManager->destroySceneNode(mStaticShapeNode);
    }

    void ChunkFactory::reposition(){
        //As of right now I'm re-checking the whole scene.
        //This was the only thing that worked for whatever reason...
        //I'm not even sure that's that bad of a way to do it, so I'll leave it there for now.
        //If not it might be a good future optimisation.
        mSceneManager->notifyStaticDirty(mSceneManager->getRootSceneNode());
    }

    void ChunkFactory::startRecipeJob(RecipeData* data, int targetIndex){
        mRunningRecipeJobs[targetIndex] = JobDispatcher::dispatchJob(new RecipeOgreMeshJob(data));
    }

    bool ChunkFactory::deconstructChunk(Chunk* chunk){
        if(!chunk) return false;

        Ogre::SceneNode *node = chunk->getStaticMeshNode();

        node->removeAndDestroyAllChildren();
        return true;
    }

    Chunk* ChunkFactory::constructChunk(const RecipeData &recipe, bool position){
        if(!recipe.recipeReady){
            AV_WARN("The chunk factory can't construct an unfinished chunk {}", recipe.coord);
            return 0;
        }

        AV_INFO("Constructing chunk {}", recipe.coord);

        Ogre::SceneNode *parentNode = mStaticShapeNode->createChildSceneNode(Ogre::SCENE_STATIC);

        if(recipe.ogreMeshData){
            for(const OgreMeshRecipeData &i : *recipe.ogreMeshData){
                Ogre::SceneNode *node = parentNode->createChildSceneNode(Ogre::SCENE_STATIC);

                Ogre::Item *item = mSceneManager->createItem(i.meshName+".mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_STATIC);
                node->attachObject((Ogre::MovableObject*)item);

                node->setPosition(i.pos);
                node->setScale(i.scale);
            }
        }

        //TODO this stuff should probably be done by the chunks themselves, rather than here, at least the things other than meshes.
        parentNode->setVisible(false);
        if(position){
            //This will eventually be an optimisation.
            //i.e creating physics shapes at their destination rather than at the origin and then having to shift them later.
            SlotPosition pos(recipe.coord.chunkX(), recipe.coord.chunkY());
            parentNode->setPosition(pos.toOgre());
            mSceneManager->notifyStaticDirty(parentNode);
        }

        Chunk *c = new Chunk(recipe.coord, mSceneManager, parentNode);

        return c;
    }

};
