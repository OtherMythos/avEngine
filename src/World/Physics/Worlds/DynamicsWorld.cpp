#include "DynamicsWorld.h"

#include "Threading/Thread/Physics/DynamicsWorldThreadLogic.h"

#include "Logger/Log.h"

namespace AV{
    DynamicsWorld* DynamicsWorld::_dynWorld;

    DynamicsWorld::DynamicsWorld(){
        _dynWorld = this;

    }

    DynamicsWorld::~DynamicsWorld(){
        _dynWorld = 0;
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

    DynamicsWorld::RigidBodyPtr DynamicsWorld::createRigidBody(const btRigidBody::btRigidBodyConstructionInfo& info){
        /// Create Dynamic Objects
        btTransform startTransform;
        startTransform.setIdentity();

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (info.m_mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if(isDynamic)
            info.m_collisionShape->calculateLocalInertia(info.m_mass, localInertia);

        //startTransform.setOrigin(btVector3(2, 10, 0));

        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        //TODO figure out what motion state actually means for what I'm trying to do.
        //btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
        //btRigidBody::btRigidBodyConstructionInfo rbInfo(info.m_mass, myMotionState, shape.get(), localInertia);

        btRigidBody *bdy = new btRigidBody(info);

        void* val = mBodyData.storeEntry(bdy);

        RigidBodyPtr sharedPtr = RigidBodyPtr(val, [](void* v) {
            //Here val isn't actually a valid pointer, so the custom deleter doesn't need to delete anything.
            //Really this is just piggy-backing on the reference counting done by the shared pointers.
            DynamicsWorld::_destroyBody(v);

            //TODO the rigid body does still need to be deleted somewhere. Figure out where that's going to be.
        });

        return sharedPtr;
    }

    void DynamicsWorld::_destroyBody(void* body){
        if(!_dynWorld) return;

        _dynWorld->mBodyData.removeEntry(body);
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

    void DynamicsWorld::_resetBufferEntries(btRigidBody* b){
        for(DynamicsWorldThreadLogic::objectCommandBufferEntry& e : mDynLogic->inputObjectCommandBuffer){
            if(e.type == DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_NONE
                || e.body == b) continue;

            //Here we can assume a match was found.
            //I just invalidate the entry rather than deleting it, as it's going to be more efficient than shifting the vector.
            e.type = DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_NONE;
            e.body = 0;
        }
    }

    void DynamicsWorld::addBody(DynamicsWorld::RigidBodyPtr body){
        std::unique_lock<std::mutex> dynamicWorldLock(dynWorldMutex);
        if(!mDynLogic) return;

        btRigidBody* b = mBodyData.getEntry(body.get());
        if(mBodiesInWorld.find(b) != mBodiesInWorld.end()) return;

        mBodiesInWorld.insert(b);

        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->inputBufferMutex);


        //Do a search for any entries in the buffer with the same pointer and invalidate them.
        _resetBufferEntries(b);
        mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_ADD, b});
    }
}
