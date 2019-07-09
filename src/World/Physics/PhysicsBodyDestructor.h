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
        PhysicsBodyDestructor();
        ~PhysicsBodyDestructor();

        /**
        Destroy a rigid body.

        Called from shared pointers, so has to be static.
        */
        static void destroyRigidBody(btRigidBody* bdy);

        void setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic);

        bool worldEventReceiver(const Event &e);

    private:
        DynamicsWorldThreadLogic* mDynLogic;

        static PhysicsBodyDestructor* mBodyDestructor;
    };
}
