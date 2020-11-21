#pragma once

#include "PhysicsTypes.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "World/Slot/Recipe/PhysicsBodyRecipeData.h"

#include "System/Util/DataPacker.h"

class btHeightfieldTerrainShape;

namespace AV{
    struct RecipeData;
    typedef unsigned int CollisionInternalId;

    /**
    A class to manage construction and destruction of physics bodies.
    */
    class PhysicsBodyConstructor{
    public:
        PhysicsBodyConstructor() = delete;
        ~PhysicsBodyConstructor() = delete;

        static void setup();
        static void shutdown();

        static PhysicsTypes::RigidBodyPtr createRigidBody(btRigidBody::btRigidBodyConstructionInfo& info, PhysicsTypes::ShapePtr shape);
        static PhysicsTypes::ShapePtr getBodyShape(void* body);

        static PhysicsTypes::CollisionObjectPtr createCollisionObject(PhysicsTypes::ShapePtr shape, CollisionPackedInt data = 0, void* dataId = 0, btVector3 origin = btVector3(0, 0, 0));

        static PhysicsTypes::PhysicsChunkEntry createPhysicsChunk(const RecipeData& recipe);
        static PhysicsTypes::CollisionChunkEntry createCollisionChunk(const RecipeData& data);

        //Create a terrain body.
        static btRigidBody* createTerrainBody(btHeightfieldTerrainShape* terrainShape);

        static void _destroyRigidBody(void* body);
        static void _destroyCollisionObject(void* object);

        static void releaseCollisionObjectInternalId(CollisionInternalId id);

    private:
        static DataPacker<PhysicsTypes::RigidBodyEntry> mBodyData;
        static DataPacker<PhysicsTypes::CollisionObjectEntry> mCollisionData;

        /**
        Set a shape's pointer to appear as attached.
        */
        static void _setShapeAttached(btCollisionShape* shape);

        static CollisionInternalId _getCollisionObjectInternalId();

        static void _createChunkShapes(const std::vector<PhysicsShapeRecipeData>& physicsShapeData, std::vector<PhysicsTypes::ShapePtr>* outShapeData);
        static btCollisionObject* _createCollisionObject(PhysicsTypes::ShapePtr shape, CollisionPackedInt data = 0, void* dataId = 0, btVector3 origin = btVector3(0, 0, 0));
    };
}
