#pragma once

#include <set>
#include <vector>

#include "World/Physics/PhysicsTypes.h"
#include "System/EnginePrerequisites.h"

#include <mutex>

class btRigidBody;
class btCollisionShape;

namespace AV{
    class Event;
    class DynamicsWorldThreadLogic;
    class CollisionWorldThreadLogic;

    /**
    A class to manage the destruction of phyics bodies.
    */
    class PhysicsBodyDestructor{
    public:
        PhysicsBodyDestructor() = delete;
        ~PhysicsBodyDestructor() = delete;

        /**
        Destroy a rigid body.

        Called from shared pointers, so has to be static.
        */
        static void destroyRigidBody(btRigidBody* bdy);

        static void destroyPhysicsWorldChunk(PhysicsTypes::PhysicsChunkEntry chunk);

        static void destroyCollisionShape(btCollisionShape *shape);

        static void destroyTerrainBody(btRigidBody* bdy);

        static void destroyCollisionObject(btCollisionObject* object);

        static void setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic);
        static void setCollisionWorldThreadLogic(uint8 worldId, CollisionWorldThreadLogic* collisionLogic);

        static bool worldEventReceiver(const Event &e);

        /**
        Update the destructor to check for any bodies or other objects that need destructing.
        */
        static void update();

        static void setup();
        static void shutdown();

    public:
        /*
        The destruction buffer for the dynamics world is stored in the DynamicsWorldThreadLogic class.
        However, for the collision stuff there can be multiple worlds, so either the destructor checks manually each world's list, or I just have a single list here.
        */

        enum class CollisionObjectDestructionType{
            DESTRUCTION_TYPE_OBJECT,
            DESTRUCTION_TYPE_CHUNK
        };

        struct CollisionOutputDestructionBufferEntry{
            btCollisionObject* obj;
            CollisionObjectDestructionType type;
        };
        static std::vector<CollisionOutputDestructionBufferEntry> mCollisonObjectDestructionBuffer;
        static std::mutex mCollisionObjectDestructionBufferMutex;

    private:
        static DynamicsWorldThreadLogic* mDynLogic;
        static CollisionWorldThreadLogic* mCollisionLogic[MAX_COLLISION_WORLDS];

        //Bodies are pending destruction until their removal is confirmed by the dynamics world.
        //Until this point they cannot be destroyed, as there is a chance they're still in the world on the separate thread, and the thread hasn't got round to destroying it yet.
        //Say for instance however, the world was destroyed in the mean time. There would be no threaded world to confirm the destruction.
        //In that case everything in the pending list can just be destroyed immediately.
        static std::set<btRigidBody*> mPendingBodies;

        static std::set<btCollisionShape*> mPendingShapes;
        static std::set<btCollisionObject*> mPendingCollisionObjects;

        static bool _shapeEverAttached(btCollisionShape *shape);

        static bool mWorldRecentlyDestroyed;

        /**
        When the world is destroyed, objects need to wait for receipt of the fact that the world on the thread has actually been destroyed.
        This boolean keeps track of whether the system is waiting for this receipt.
        */
        static bool mWorldDestructionPending;

        /**
        Check if the thread has gotten around to destroying the world, and whether any objects waiting on the world need to be destroyed.
        */
        static void _checkWorldDestructionReceipt();

        /**
        Clear all the pending objects.
        */
        static void _clearState();

        static void _destroyRigidBody(btRigidBody* bdy);
        static void _destroyCollisionShape(btCollisionShape* shape);
        static void _destroyCollisionObject(btCollisionObject* object);

        static void _checkDynamicsWorldDestructables();
        static void _checkCollisionWorldDestructables();
    };
}
