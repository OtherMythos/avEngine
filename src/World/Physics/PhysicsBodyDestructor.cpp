#include "PhysicsBodyDestructor.h"

#include "btBulletDynamicsCommon.h"

#include "Threading/Thread/Physics/DynamicsWorldThreadLogic.h"
#include "World/Physics/Worlds/DynamicsWorldMotionState.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"


namespace AV{

    DynamicsWorldThreadLogic* PhysicsBodyDestructor::mDynLogic = 0;
    std::set<btRigidBody*> PhysicsBodyDestructor::mPendingBodies;
    std::set<btCollisionShape*> PhysicsBodyDestructor::mPendingShapes;


    void PhysicsBodyDestructor::setup(){
        EventDispatcher::subscribeStatic(EventType::World, AV_BIND_STATIC(PhysicsBodyDestructor::worldEventReceiver));
    }

    void PhysicsBodyDestructor::shutdown(){
        //The engine is shutting down, so any bodies pending for destruction can now just be destroyed.
        _clearState();
    }

    void PhysicsBodyDestructor::_clearState(){
        for(btRigidBody* b : mPendingBodies){
            _destroyRigidBody(b);
        }
        for(btCollisionShape* s : mPendingShapes){
            delete s;
        }
        mPendingBodies.clear();
        mPendingShapes.clear();
    }

    void PhysicsBodyDestructor::destroyRigidBody(btRigidBody* bdy){

        if(mDynLogic){
            std::unique_lock<std::mutex> inputBufferLock(mDynLogic->objectInputBufferMutex);

            //Request the body for destruction.
            mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_DESTROY, bdy});

            mPendingBodies.insert(bdy);
        }else{
            //If there is no dynamic logic, we can assume there's no world. So destruction can happen immediately.

            _destroyRigidBody(bdy);
            //No need to clear the pending list, because this body is being destroyed immediately it would never make it in there anyway.
        }
    }

    void PhysicsBodyDestructor::destroyCollisionShape(btCollisionShape *shape){

        //No mDynLogic means there is no world, so just delete the shapes.
        if(mDynLogic && _shapeEverAttached(shape)){
            mPendingShapes.insert(shape);
        }else{
            //If the shape was never attached to anything, we can just delete it.
            //If the world doesn't exist we can also just delete it.
            delete shape;
        }
    }

    bool PhysicsBodyDestructor::_shapeEverAttached(btCollisionShape *shape){
        //Take the shape pointer as a 32 bit number, and take the final bit of that to indicate whether a shape was ever attached.
        return ((uintptr_t)shape->getUserPointer() & 0x80000000) >> 31;
    }

    void PhysicsBodyDestructor::update(){
        if(!mDynLogic) return; //Nothing to check

        //Check the output buffer for bodies that can be destroyed.
        {
            std::unique_lock<std::mutex> outputDestructionLock(mDynLogic->outputDestructionBufferMutex);

            for(const DynamicsWorldThreadLogic::OutputDestructionBufferEntry& entry : mDynLogic->outputDestructionBuffer){
                switch(entry.type){
                    case DynamicsWorldThreadLogic::ObjectDestructionType::DESTRUCTION_TYPE_BODY: {
                        btCollisionShape* s = entry.body->getCollisionShape();
                        if(mPendingShapes.find(s) != mPendingShapes.end()){
                            delete s;
                            mPendingShapes.erase(s);
                        }
                        _destroyRigidBody(entry.body);
                        mPendingBodies.erase(entry.body);
                    }
                };
            }

            mDynLogic->outputDestructionBuffer.clear();
        }
    }

    void PhysicsBodyDestructor::_destroyRigidBody(btRigidBody* bdy){
        DynamicsWorldMotionState* motionState = (DynamicsWorldMotionState*)bdy->getMotionState();
        if(motionState){
            delete motionState;
        }
        delete bdy;
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
        else if(event.eventCategory() == WorldEventCategory::Destroyed){
            //If the world is about to be destroyed then the pending list can be cleared.
            _clearState();
        }

        return false;
    }
}
