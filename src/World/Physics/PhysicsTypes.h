#pragma once

#include <vector>
#include <memory>

class btRigidBody;
class btCollisionShape;
class btCollisionObject;

namespace AV{
    namespace PhysicsTypes{
        typedef std::shared_ptr<btCollisionShape> ShapePtr;

        //TODO rename this to something without the entry in the name
        typedef std::pair<std::vector<ShapePtr>*, std::vector<btRigidBody*>*> PhysicsChunkEntry;

        typedef std::shared_ptr<void> RigidBodyPtr;
        typedef std::shared_ptr<void> CollisionSenderPtr;

        typedef std::pair<btRigidBody*, PhysicsTypes::ShapePtr> RigidBodyEntry;
        typedef std::pair<btCollisionObject*, PhysicsTypes::ShapePtr> CollisionObjectEntry;


        static const PhysicsChunkEntry EMPTY_CHUNK_ENTRY;
    }
};
