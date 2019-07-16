#pragma once

#include "PhysicsTypes.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"

//TODO this is here on a trial basis. If it works out the name and class should be re-named and put somewhere else.
#include "Scripting/ScriptDataPacker.h"

namespace AV{
    class PhysicsBodyRecipeData;
    class PhysicsShapeRecipeData;

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

        static PhysicsTypes::PhysicsChunkEntry createPhysicsChunk(const std::vector<PhysicsBodyRecipeData>& physicsBodyData, const std::vector<PhysicsShapeRecipeData>& physicsShapeData);

        static void _destroyRigidBody(void* body);

    private:
        static ScriptDataPacker<PhysicsTypes::RigidBodyEntry> mBodyData;

        /**
        Set a shape's pointer to appear as attached.
        */
        static void _setShapeAttached(btCollisionShape* shape);
    };
}
