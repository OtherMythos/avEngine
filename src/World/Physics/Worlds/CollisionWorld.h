#pragma once

#include "System/EnginePrerequisites.h"
#include "PhysicsWorld.h"

#include "Scripting/ScriptDataPacker.h"
#include "CollisionWorldUtils.h"

namespace AV{
    class CollisionWorldThreadLogic;
    class PhysicsBodyConstructor;

    class CollisionWorld : public PhysicsWorld{
        friend PhysicsBodyConstructor;
    public:
        typedef uint8 CollisionWorldId;

        CollisionWorld(CollisionWorldId id);
        ~CollisionWorld();

        enum CollisionFunctionStatus{
            SUCCESS = 0,
            ALREADY_IN_WORLD = 1,
            NOT_IN_WORLD,
            NO_WORLD,
            WRONG_WORLD
        };

        void update();

        void notifyOriginShift(const Ogre::Vector3 &offset, const SlotPosition& newPos);

        CollisionFunctionStatus addObject(PhysicsTypes::CollisionObjectPtr object);
        CollisionFunctionStatus removeObject(PhysicsTypes::CollisionObjectPtr object);

        void setCollisionWorldThreadLogic(CollisionWorldThreadLogic* threadLogic);

        CollisionFunctionStatus setObjectPosition(PhysicsTypes::CollisionObjectPtr object, const btVector3& pos);

        //Should only be called at shared pointer destruction.
        static void _removeObject(const btCollisionObject* object);

        static uint8 getObjectWorld(PhysicsTypes::CollisionObjectPtr object);
        static CollisionObjectType::CollisionObjectType getObjectType(PhysicsTypes::CollisionObjectPtr object);

        /**
        Insert a collision object chunk into the world.
        This function is static, as it targets all existing worlds.
        */
        static uint32 addCollisionObjectChunk(const PhysicsTypes::CollisionChunkEntry& chunk);

        /**
        Set the position of the object, performing world id checks as part of this function.
        */
        static void setObjectPositionStatic(PhysicsTypes::CollisionObjectPtr object, const btVector3& pos);

    private:
        CollisionWorldId mWorldId;

        CollisionWorldThreadLogic* mThreadLogic = 0;

        static ScriptDataPacker<PhysicsTypes::CollisionObjectEntry>* mCollisionObjectData;

        CollisionWorld::CollisionFunctionStatus _setPositionInternal(btCollisionObject* obj, const btVector3& pos);

        struct CollisionEventEntry{
            //In future this could just be changed to ints representing the values.
            const btCollisionObject* sender;
            const btCollisionObject* receiver;
            CollisionObjectEventMask::CollisionObjectEventMask eventMask;
        };
        //A thread side list of the collision events.
        std::vector<CollisionEventEntry> mCollisionEvents;
        std::set<const btCollisionObject*> mObjectsInWorld;

        /**
        Clear a value from the thread logic's input buffer.
        This function should only be called when the mutex is held for the collision world.
        */
        void _resetBufferEntries(const btCollisionObject* o);
        inline bool _objectInWorld(const btCollisionObject* bdy) const;
    };
}
