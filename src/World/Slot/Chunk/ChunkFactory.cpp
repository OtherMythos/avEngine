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
#include "Threading/Jobs/RecipeSceneJob.h"
#include "Threading/Jobs/RecipePhysicsBodiesJob.h"
#include "Threading/Jobs/RecipeCollisionObjectsJob.h"
#include "Threading/Jobs/RecipeChunkMetaJob.h"
#include "Threading/Jobs/RecipeNavMeshJob.h"
#include "Threading/Jobs/RecipeDataPointJob.h"
#include "System/SystemSetup/SystemSetup.h"
#include "Scripting/Event/SystemEventListenerObjects.h"

#include "Terrain/Terrain.h"
#include "TerrainManager.h"

namespace AV{
    ChunkFactory::ChunkFactory(std::shared_ptr<PhysicsManager> physicsManager, std::shared_ptr<TerrainManager> terrainManager, std::shared_ptr<NavMeshManager> navMeshManager)
        : mPhysicsManager(physicsManager),
          mTerrainManager(terrainManager),
          mNavMeshManager(navMeshManager) {

    }

    void ChunkFactory::initialise(){
        mSceneManager = Ogre::Root::getSingletonPtr()->getSceneManager("Scene Manager");

        mStaticShapeNode = mSceneManager->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_STATIC);
        mStaticShapeNode->setPosition(Ogre::Vector3::ZERO);
    }

    ChunkFactory::~ChunkFactory(){
        if(mTerrainManager) mTerrainManager->destroyTerrains();
    }

    void ChunkFactory::shutdown(){
        for(int i  = 0; i < RecipeData::MaxRecipies; i++){
            //AV_INFO("Waiting for job {} in shutdown", mRunningMeshJobs[i].id());
            JobDispatcher::endJob(mRunningMeshJobs[i]);
            JobDispatcher::endJob(mRunningBodyJobs[i]);
            JobDispatcher::endJob(mCollisionObjectsJobs[i]);
            JobDispatcher::endJob(mRunningNavMeshJobs[i]);
            JobDispatcher::endJob(mRunningChunkMetaJobs[i]);
            JobDispatcher::endJob(mRunningDataPointJobs[i]);
        }

        if(mStaticShapeNode) mSceneManager->destroySceneNode(mStaticShapeNode);
    }

    void ChunkFactory::startRecipeJob(RecipeData* data, int targetIndex){
        mRunningMeshJobs[targetIndex] = JobDispatcher::dispatchJob(new RecipeSceneJob(data));
        mRunningNavMeshJobs[targetIndex] = JobDispatcher::dispatchJob(new RecipeNavMeshJob(data));
        mRunningChunkMetaJobs[targetIndex] = JobDispatcher::dispatchJob(new RecipeChunkMetaJob(data));
        mRunningDataPointJobs[targetIndex] = JobDispatcher::dispatchJob(new RecipeDataPointJob(data));

        if(SystemSettings::getDynamicPhysicsDisabled()){
            //Mark the job as done, because it's never actually going to run.
            data->jobDoneCounter++;
        }else{
            mRunningBodyJobs[targetIndex] = JobDispatcher::dispatchJob(new RecipePhysicsBodiesJob(data));
        }

        if(SystemSettings::getNumCollisionWorlds() <= 0){
            data->jobDoneCounter++;
        }else{
            mCollisionObjectsJobs[targetIndex] = JobDispatcher::dispatchJob(new RecipeCollisionObjectsJob(data));
        }
    }

    void ChunkFactory::_destroyNode(Ogre::SceneNode* node){
        auto it = node->getChildIterator();
        while (it.hasMoreElements()) {
            Ogre::SceneNode *eNode = (Ogre::SceneNode*)it.getNext();

            Ogre::SceneNode::ObjectIterator objIt = eNode->getAttachedObjectIterator();
            while(objIt.hasMoreElements()){
                Ogre::MovableObject* object = static_cast<Ogre::MovableObject*>(objIt.getNext());
                mSceneManager->destroyMovableObject(object);
            }

            _destroyNode(eNode);
            eNode->removeAndDestroyAllChildren();
        }
    }

    bool ChunkFactory::deconstructChunk(Chunk* chunk){
        if(!chunk) return false;

        Ogre::SceneNode *node = chunk->getStaticMeshNode();

        _destroyNode(node);

        node->removeAndDestroyAllChildren();

        Terrain* t = chunk->getTerrain();
        if(t){
            t->teardown();
            mTerrainManager->releaseTerrain(t); //Release the terrain so some other chunk can use it at a later date.
        }

        return true;
    }

    Chunk* ChunkFactory::constructChunk(const RecipeData &recipe, bool position){
        if(!recipe.recipeReady){
            AV_WARN("The chunk factory can't construct an unfinished chunk {}", recipe.coord);
            return 0;
        }

        AV_INFO("Constructing chunk {}", recipe.coord);

        SystemEventListenerObjects::mCurrentRecipeData = &recipe;
        SystemEventListenerObjects::executeListener(SystemEventListenerObjects::CHUNK);
        SystemEventListenerObjects::mCurrentRecipeData = 0;

        Ogre::SceneNode *parentNode = mStaticShapeNode->createChildSceneNode(Ogre::SCENE_STATIC);

        /*if(recipe.ogreMeshData){
            for(const OgreMeshRecipeData &i : *recipe.ogreMeshData){
                Ogre::SceneNode *node = parentNode->createChildSceneNode(Ogre::SCENE_STATIC);

                Ogre::Item *item = mSceneManager->createItem(i.meshName+".mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_STATIC);
                node->attachObject((Ogre::MovableObject*)item);

                node->setPosition(i.pos);
                node->setScale(i.scale);
            }
        }*/
        if(recipe.sceneEntries){
            int currentMesh = 0;
            int entries = _createSceneTree(recipe, 0, parentNode, currentMesh);
            assert(entries == recipe.sceneEntries->size()); //Should have reached the end.
            assert(currentMesh == recipe.ogreMeshData->size());
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
            physicsChunk = PhysicsBodyConstructor::createPhysicsChunk(recipe);
        }

        PhysicsTypes::CollisionChunkEntry collisionChunk = PhysicsTypes::EMPTY_COLLISION_CHUNK_ENTRY;
        if(
            recipe.collisionData.collisionObjectPackedData &&
            recipe.collisionData.collisionScriptData &&
            recipe.collisionData.collisionShapeData &&
            recipe.collisionData.collisionScriptAndClosures &&
            recipe.collisionData.collisionObjectRecipeData
        ){
            //Construct the collision objects chunk.
            collisionChunk = PhysicsBodyConstructor::createCollisionChunk(recipe);
        }

        //Create the terrain.
        Terrain* t = 0;
        if(recipe.chunkSettings.terrainEnabled){
            Ogre::SceneNode *terrainNode = mStaticShapeNode->createChildSceneNode(Ogre::SCENE_STATIC);
            terrainNode->setVisible(true);

            t = mTerrainManager->requestTerrain();

            SlotPosition pos(recipe.coord.chunkX(), recipe.coord.chunkY());
            terrainNode->setPosition(pos.toOgre());
            mSceneManager->notifyStaticDirty(terrainNode);

            t->provideSceneNode(terrainNode);
            bool setupSuccess = t->setup(recipe.coord, *mTerrainManager);
            if(!setupSuccess){
                //There was a problem loading this terrain, most likely that the map data isn't correct.
                //OPTIMISTION this could be improved by having a static terrain function to check if it's valid.
                //This would be called before the terrain loading starts to avoid this request and release cycle.
                mTerrainManager->releaseTerrain(t);
                t = 0;
            }
        }

        Chunk *c = new Chunk(recipe.coord, mPhysicsManager, mNavMeshManager, mSceneManager, parentNode, physicsChunk, collisionChunk, t, recipe.loadedNavMesh);

        return c;
    }

    void ChunkFactory::getTerrainTestData(int& inUseTerrains, int& availableTerrains){
        mTerrainManager->getTerrainTestData(inUseTerrains, availableTerrains);
    }

    int ChunkFactory::_createSceneTree(const RecipeData &recipe, int currentNode, Ogre::SceneNode* parentNode, int& currentMesh){
        Ogre::SceneNode* previousNode = 0;
        while(currentNode < recipe.sceneEntries->size()){
            const RecipeSceneEntry& e = (*recipe.sceneEntries)[currentNode];

            if(e.type == SceneType::child){
                currentNode = _createSceneTree(recipe, currentNode + 1, previousNode, currentMesh);
            }else if(e.type == SceneType::term){
                return currentNode + 1;
            }else{
                previousNode = parentNode->createChildSceneNode(Ogre::SCENE_STATIC);

                if(e.type == SceneType::mesh){
                    //Create a mesh.
                    //TODO to improve this I should build up a dictionary of mesh names.
                    //If I have duplicate items with the same mesh, the string would end up being stored twice.
                    //So instead I should build up a section of mesh names at the beginning, and then a list of ids.
                    const std::string& targetMesh = (*recipe.ogreMeshData)[currentMesh].meshName;
                    Ogre::Item *item = mSceneManager->createItem(targetMesh, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_STATIC);
                    previousNode->attachObject((Ogre::MovableObject*)item);
                    currentMesh++;
                }

                previousNode->setPosition(e.pos);
                previousNode->setScale(e.scale);
                previousNode->setOrientation(e.orientation);
                currentNode++;
            }
        }

        return currentNode;
    }

};
