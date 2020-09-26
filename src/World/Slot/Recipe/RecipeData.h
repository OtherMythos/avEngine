#pragma once

#include "World/Slot/ChunkCoordinate.h"
#include "OgreMeshRecipeData.h"
#include "PhysicsBodyRecipeData.h"

#include <atomic>
#include <vector>
#include <OgreVector3.h>

class dtNavMesh;

namespace AV{
    //TODO maybe put these into a different file.
    enum class SceneType{
        child,
        empty,
        term,
        mesh,
        staticPhysicsShape,
        collisionSender
    };

    struct RecipeSceneEntry{
        SceneType type;
        unsigned int id;
        Ogre::Vector3 pos;
        Ogre::Vector3 scale;
        Ogre::Quaternion orientation;
    };

    struct CollisionWorldChunkData{
        //Collision world
        //Vector of packed collision world ints. This pointer can be null if parsing failed.
        std::vector<CollisionObjectPropertiesData> *collisionObjectPackedData = 0;
        //References to the strings list a combination of script and closure.
        std::vector<CollisionObjectScriptData> *collisionScriptData = 0;
        //Collision physics shape data.
        std::vector<PhysicsShapeRecipeData> *collisionShapeData = 0;
        //Collision information for scripts and closures. Both scripts and closures strings are pushed to this list, and collisionClosuresBegin is used to spcify the split.
        std::vector<std::string> *collisionScriptAndClosures = 0;
        //List of entries, one for each object to insert.
        std::vector<CollisionObjectRecipeData> *collisionObjectRecipeData = 0;
        //The index into the collisionScriptAndClosures vector where the definitions of scripts ends and closures begin.
        uint16 collisionClosuresBegin = 0;
    };

    struct RecipeData{
        RecipeData() : jobDoneCounter(0) { };

        ChunkCoordinate coord = ChunkCoordinate();

        //Whether this slot can be used to create a recipe.
        bool slotAvailable = true;

        //Whether or not this recipe has had all its loading jobs completed and is ready to be constructed.
        bool recipeReady = false;

        //Score used to keep track of the age of the recipe. This is used by the SlotManager to determine which slots are the oldest.
        int recipeScore = 0;

        //Counter to determine how done a recipe is. This will be incremented by the jobs. When this is equal to the targetJobs the SlotManager will mark this recipe as ready.
        std::atomic<int> jobDoneCounter;

        //The parsed scene tree entries.
        std::vector<RecipeSceneEntry>* sceneEntries = 0;

        //Data of the static ogre meshes which should be inserted into the scene.
        std::vector<OgreMeshRecipeData>* ogreMeshData = 0;

        //Dynamics world
        //Vector pointer to the physics body data constructed by the thread job. This might be 0 if the job failed.
        std::vector<PhysicsBodyRecipeData> *physicsBodyData = 0;
        //Recipe data for shapes that should be created.
        std::vector<PhysicsShapeRecipeData> *physicsShapeData = 0;

        CollisionWorldChunkData collisionData;

        /*
        In future this will be changed to potentially a list of mesh tiles.
        */
        dtNavMesh* loadedNavMesh = 0;

        //The number of jobs a recipe contains.
        //Currently includes meshes, physics shapes, collision objects, nav meshes.
        static const int targetJobs = 4;
        static const int MaxRecipies = 10;
    };
}
