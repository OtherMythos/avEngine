#include "PhysicsBodyDestructor.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"

namespace AV{
    PhysicsBodyDestructor* PhysicsBodyDestructor::mBodyDestructor = 0;

    PhysicsBodyDestructor::PhysicsBodyDestructor(){
        mBodyDestructor = this;

        EventDispatcher::subscribe(EventType::World, AV_BIND(PhysicsBodyDestructor::worldEventReceiver));
    }

    PhysicsBodyDestructor::~PhysicsBodyDestructor(){
        //Should only be destroyed on engine shutdown.
        mBodyDestructor = 0;
    }

    void PhysicsBodyDestructor::destroyRigidBody(btRigidBody* bdy){

    }

    void PhysicsBodyDestructor::setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic){
        mDynLogic = dynLogic;
    }

    bool PhysicsBodyDestructor::worldEventReceiver(const Event &e){
        const WorldEvent& event = (WorldEvent&)e;

        if(event.eventCategory() == WorldEventCategory::Created){
            const WorldEventCreated& wEvent = (WorldEventCreated&)event;
            //The destruction might need to keep a reference to the dynamics world, so it can confirm with it that shapes were removed from the list.
            //However, I'm not sure this is necessary, so it's commented out until I've figured that out!
            //mDynWorld = wEvent->getPhysicsManager()->getDynamicsWorld();
        }

        return false;
    }
}
