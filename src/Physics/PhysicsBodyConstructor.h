#pragma once

#include "PhysicsTypes.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"

#include "System/Util/DataPacker.h"

class btHeightfieldTerrainShape;

namespace AV{

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

        static PhysicsTypes::CollisionObjectPtr createCollisionObject(PhysicsTypes::ShapePtr shape, CollisionPackedInt data = 0, void* dataId = 0, btVector3 origin = btVector3(0, 0, 0), btQuaternion orientation = btQuaternion::getIdentity());

        //Create a terrain body.
        static btRigidBody* createTerrainBody(btHeightfieldTerrainShape* terrainShape);

        static void _destroyRigidBody(void* body);
        static void _destroyCollisionObject(void* object);

    private:
        static DataPacker<PhysicsTypes::RigidBodyEntry> mBodyData;
        static DataPacker<PhysicsTypes::CollisionObjectEntry> mCollisionData;

        /**
        Set a shape's pointer to appear as attached.
        */
        static void _setShapeAttached(btCollisionShape* shape);
        static btCollisionObject* _createCollisionObject(PhysicsTypes::ShapePtr shape, CollisionPackedInt data, void* dataId, const btVector3& origin, const btQuaternion& orientation);
    };
}
