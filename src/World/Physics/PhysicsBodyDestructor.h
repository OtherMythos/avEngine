#pragma once

#include <set>
#include <vector>

class btRigidBody;
class btCollisionShape;

namespace AV{
    class Event;
    class DynamicsWorldThreadLogic;

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

        static void destroyCollisionShape(btCollisionShape *shape);

        static void setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic);

        static bool worldEventReceiver(const Event &e);

        /**
        Update the destructor to check for any bodies or other objects that need destructing.
        */
        static void update();

        static void setup();
        static void shutdown();

    private:
        static DynamicsWorldThreadLogic* mDynLogic;

        //Bodies are pending destruction until their removal is confirmed by the dynamics world.
        //Until this point they cannot be destroyed, as there is a chance they're still in the world on the separate thread, and the thread hasn't got round to destroying it yet.
        //Say for instance however, the world was destroyed in the mean time. There would be no threaded world to confirm the destruction.
        //In that case everything in the pending list can just be destroyed immediately.
        static std::set<btRigidBody*> mPendingBodies;

        static std::set<btCollisionShape*> mPendingShapes;

        static bool _shapeEverAttached(btCollisionShape *shape);

        static bool mWorldRecentlyDestroyed;

        /**
        Clear all the pending objects.
        */
        static void _clearState();

        static void _destroyRigidBody(btRigidBody* bdy);
    };
}
