#pragma once

//TODO I'd rather not have to enclude the entire shape manager for the sake of a single pointer.
//I feel at some point the shape ptr should be taken out of there and put somewhere else.
#include "World/Physics/PhysicsShapeManager.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"

//TODO this is here on a trial basis. If it works out the name and class should be re-named and put somewhere else.
#include "Scripting/ScriptDataPacker.h"

namespace AV{
    /**
    A class to manage construction and destruction of physics bodies.
    */
    class PhysicsBodyConstructor{
    public:
        PhysicsBodyConstructor();
        ~PhysicsBodyConstructor();

        typedef std::pair<btRigidBody*, PhysicsShapeManager::ShapePtr> RigidBodyEntry;
        typedef std::shared_ptr<void> RigidBodyPtr;

        RigidBodyPtr createRigidBody(btRigidBody::btRigidBodyConstructionInfo& info, PhysicsShapeManager::ShapePtr shape);
        PhysicsShapeManager::ShapePtr getBodyShape(void* body);

        static void _destroyRigidBody(void* body);

    private:
        ScriptDataPacker<RigidBodyEntry> mBodyData;

        static PhysicsBodyConstructor* _bodyConstructor;
    };
}
