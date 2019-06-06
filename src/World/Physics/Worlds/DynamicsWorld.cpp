#include "DynamicsWorld.h"

#include "Threading/Thread/Physics/DynamicsWorldThreadLogic.h"

#include "Logger/Log.h"

namespace AV{
    DynamicsWorld::DynamicsWorld(){

    }

    DynamicsWorld::~DynamicsWorld(){

    }

    void DynamicsWorld::update(){
        std::unique_lock<std::mutex> outputBufferLock(mDynLogic->outputBufferMutex);

        //Check if there's anything in the command buffer that needs addressing.
        //This isn't guaranteed to contain anything, for example if the current physics processing is taking a long time.
        if(mDynLogic->outputBuffer.size() > 0){
            for(const DynamicsWorldThreadLogic::outputBufferEntry& entry : mDynLogic->outputBuffer){
                btVector3 pos = entry.pos;
                AV_INFO("new entity position {} {} {}", pos.x(), pos.y(), pos.z());
            }
        }
    }

    btRigidBody* DynamicsWorld::createRigidBody(PhysicsShapeManager::ShapePtr shape, const btRigidBody::btRigidBodyConstructionInfo& info){
        /// Create Dynamic Objects
        btTransform startTransform;
        startTransform.setIdentity();

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (info.m_mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if(isDynamic)
            shape->calculateLocalInertia(info.m_mass, localInertia);

        //startTransform.setOrigin(btVector3(2, 10, 0));

        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        //TODO figure out what motion state actually means for what I'm trying to do.
        btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(info.m_mass, myMotionState, shape.get(), localInertia);
        
        return new btRigidBody(rbInfo);
    }

    void DynamicsWorld::setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic){
        //TODO this mutex has to be duplicated each time I want to do something with the thread. I don't like that.
        //TODO BUG!!!! when this mutex is actually given a name (meaning it gets applied), add body locks up by trying to claim the mutex.
        //Ultimately add body shouldn't appear here, and it's a work in progress to write the infrustructure to remove it.
        std::unique_lock<std::mutex> (dynWorldMutex);

        mDynLogic = dynLogic;
        //TODO get rid of this when able.
        //addBody(tmpBody);
    }

    void DynamicsWorld::addBody(btRigidBody* body){
        std::unique_lock<std::mutex> dynamicWorldLock(dynWorldMutex);
        if(!mDynLogic) return;

        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->inputBufferMutex);

        mDynLogic->inputBuffer.push_back({body});
    }
}
