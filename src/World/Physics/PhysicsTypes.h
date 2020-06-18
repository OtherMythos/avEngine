#pragma once

#include <vector>
#include <memory>

class btRigidBody;
class btCollisionShape;
class btCollisionObject;

namespace AV{
    /**
    A regular integer, which contains a number of relevant settings packed into it.
    Usage of this integer can be found in the CollisionWorldUtils.h file.
    */
    typedef int CollisionPackedInt;

    namespace PhysicsTypes{
        typedef std::shared_ptr<btCollisionShape> ShapePtr;

        //TODO rename this to something without the entry in the name
        typedef std::pair<std::vector<ShapePtr>*, std::vector<btRigidBody*>*> PhysicsChunkEntry;

        typedef std::shared_ptr<void> RigidBodyPtr;
        typedef std::shared_ptr<void> CollisionObjectPtr;

        typedef std::pair<btRigidBody*, PhysicsTypes::ShapePtr> RigidBodyEntry;
        typedef std::pair<btCollisionObject*, PhysicsTypes::ShapePtr> CollisionObjectEntry;


        static const PhysicsChunkEntry EMPTY_CHUNK_ENTRY;
    }
};
