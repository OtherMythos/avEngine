#pragma once

class btRigidBody;

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

        static void setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic);

        static bool worldEventReceiver(const Event &e);

    private:
        static DynamicsWorldThreadLogic* mDynLogic;

    };
}
