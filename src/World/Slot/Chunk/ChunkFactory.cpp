#include "ChunkFactory.h"

#include "World/Slot/ChunkCoordinate.h"
#include "World/Slot/SlotPosition.h"
#include "World/Slot/Recipe/OgreMeshRecipeData.h"
#include "World/Slot/Recipe/RecipeData.h"

#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"
#include "World/Physics/PhysicsBodyConstructor.h"

#include "Logger/Log.h"

#include "Chunk.h"

#include "OgreRoot.h"
#include "Threading/JobDispatcher.h"
#include "Threading/Jobs/RecipeOgreMeshJob.h"
#include "Threading/Jobs/RecipePhysicsBodiesJob.h"

namespace AV{
    ChunkFactory::ChunkFactory(std::shared_ptr<PhysicsManager> physicsManager)
        : mPhysicsManager(physicsManager) {

    }

    void ChunkFactory::initialise(){
        mSceneManager = Ogre::Root::getSingletonPtr()->getSceneManager("Scene Manager");

        mStaticShapeNode = mSceneManager->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_STATIC);
        mStaticShapeNode->setPosition(Ogre::Vector3::ZERO);
    }

    ChunkFactory::~ChunkFactory(){

    }

    void ChunkFactory::shutdown(){
        for(int i  = 0; i < RecipeData::MaxRecipies; i++){
            //AV_INFO("Waiting for job {} in shutdown", mRunningMeshJobs[i].id());
            JobDispatcher::endJob(mRunningMeshJobs[i]);
            JobDispatcher::endJob(mRunningBodyJobs[i]);
        }

        if(mStaticShapeNode) mSceneManager->destroySceneNode(mStaticShapeNode);
    }

    void ChunkFactory::startRecipeJob(RecipeData* data, int targetIndex){
        mRunningMeshJobs[targetIndex] = JobDispatcher::dispatchJob(new RecipeOgreMeshJob(data));
        mRunningBodyJobs[targetIndex] = JobDispatcher::dispatchJob(new RecipePhysicsBodiesJob(data));
    }

    bool ChunkFactory::deconstructChunk(Chunk* chunk){
        if(!chunk) return false;

        Ogre::SceneNode *node = chunk->getStaticMeshNode();

        //This also needs to destroy the objects themselves.
        auto it = node->getChildIterator();
        while (it.hasMoreElements()) {
            Ogre::SceneNode *eNode = (Ogre::SceneNode*)it.getNext();

            Ogre::MovableObject* object = eNode->getAttachedObject(0);
            mSceneManager->destroyMovableObject(object);
        }

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


        //Physics stuff
        PhysicsTypes::PhysicsChunkEntry physicsChunk = PhysicsTypes::EMPTY_CHUNK_ENTRY;
        if(recipe.physicsBodyData && recipe.physicsShapeData){
            physicsChunk = PhysicsBodyConstructor::createPhysicsChunk(*recipe.physicsBodyData, *recipe.physicsShapeData);
        }

        Chunk *c = new Chunk(recipe.coord, mPhysicsManager, mSceneManager, parentNode, physicsChunk);

        return c;
    }

};
