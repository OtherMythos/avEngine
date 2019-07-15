#pragma once

#include <vector>
#include <memory>

class btRigidBody;
class btCollisionShape;

namespace AV{
    namespace PhysicsTypes{
        typedef std::shared_ptr<btCollisionShape> ShapePtr;

        //TODO rename this to something without the entry in the name
        typedef std::pair<std::vector<ShapePtr>*, std::vector<btRigidBody*>*> PhysicsChunkEntry;


        static const PhysicsChunkEntry EMPTY_CHUNK_ENTRY;
    }
};
