#include "DynamicsWorld.h"

#include "Threading/Thread/Physics/DynamicsWorldThreadLogic.h"
#include "World/Physics/Worlds/DynamicsWorldMotionState.h"

#include "Logger/Log.h"

namespace AV{
    DynamicsWorld* DynamicsWorld::_dynWorld;

    DynamicsWorldThreadLogic* DynamicsWorldMotionState::dynLogic = 0;

    DynamicsWorld::DynamicsWorld(){
        _dynWorld = this;
    }

    DynamicsWorld::~DynamicsWorld(){
        _dynWorld = 0;
    }

    void DynamicsWorld::update(){
        std::unique_lock<std::mutex> outputBufferLock(mDynLogic->outputBufferMutex);

        mEntityTransformData.clear();

        //Check if there's anything in the command buffer that needs addressing.
        //This isn't guaranteed to contain anything, for example if the current physics processing is taking a long time.
        if(mDynLogic->outputBuffer.size() > 0){
            for(const DynamicsWorldThreadLogic::outputBufferEntry& entry : mDynLogic->outputBuffer){
                BodyAttachObjectType type = (BodyAttachObjectType) entry.body->getUserIndex();

                //A potential optimisation here would be making it so that these entries don't get pushed in the first place.
                //However that would mean I would have to read the user index on the physics thread side, which at the moment isn't possible.
                if(type == BodyAttachObjectType::OBJECT_TYPE_NONE) continue;

                switch(type){
                    case BodyAttachObjectType::OBJECT_TYPE_ENTITY:
                        eId entity = mEntitiesInWorld[entry.body];
                        mEntityTransformData.push_back({entity, entry.pos});
                        break;
                };
            }
        }
        //We've read the values, and don't want to risk reading them again, so the buffer should be cleared.
        mDynLogic->outputBuffer.clear();
    }

    DynamicsWorld::RigidBodyPtr DynamicsWorld::createRigidBody(btRigidBody::btRigidBodyConstructionInfo& info, PhysicsShapeManager::ShapePtr shape){
        /// Create Dynamic Objects
        btTransform startTransform;
        startTransform.setIdentity();

        info.m_collisionShape = shape.get();

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (info.m_mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        DynamicsWorldMotionState* motionState = new DynamicsWorldMotionState(info.m_startWorldTransform);
        if(isDynamic){
            info.m_collisionShape->calculateLocalInertia(info.m_mass, localInertia);

            //If the mass is 0, we don't need to give it a motion state as the motion state just helps to inform us when the shape has moved.
            //DynamicsWorldMotionState *motion =
            info.m_motionState = motionState;
        }

        btRigidBody *bdy = new btRigidBody(info);
        //To tell it that nothing is attached to it.
        bdy->setUserIndex(0);

        if(isDynamic){
            //The pointer to the body is used as an identifier.
            motionState->body = bdy;
        }

        //We store a copy of the pointer to the shape as well.
        //That way there's no chance of the shape being destroyed while the rigid body is still using it.
        void* val = mBodyData.storeEntry(rigidBodyEntry(bdy, shape));

        RigidBodyPtr sharedPtr = RigidBodyPtr(val, [](void* v) {
            //Here val isn't actually a valid pointer, so the custom deleter doesn't need to delete anything.
            //Really this is just piggy-backing on the reference counting done by the shared pointers.
            DynamicsWorld::_destroyBody(v);

            //TODO the rigid body does still need to be deleted somewhere. Figure out where that's going to be.
        });

        return sharedPtr;
    }

    void DynamicsWorld::_destroyBody(void* body){
        //TODO maybe think of a different way to do this than using a static pointer.
        if(!_dynWorld) return;

        //For the shape it actually needs to be destroyed manually.
        _dynWorld->mBodyData.getEntry(body).second.reset();

        _dynWorld->mBodyData.removeEntry(body);
    }

    bool DynamicsWorld::_attachToBody(btRigidBody* body, DynamicsWorld::BodyAttachObjectType type){
        //We can't attach an object that's already attached to something.
        if((BodyAttachObjectType)body->getUserIndex() != BodyAttachObjectType::OBJECT_TYPE_NONE) return false;

        //As long as the user index is only written and read by the main thread, it should be thread safe.
        body->setUserIndex((int) type);

        return true;
    }

    void DynamicsWorld::_detatchFromBody(btRigidBody* body){
        body->setUserIndex((int) BodyAttachObjectType::OBJECT_TYPE_NONE);
    }

    bool DynamicsWorld::attachEntityToBody(DynamicsWorld::RigidBodyPtr body, eId e){
        btRigidBody* b = mBodyData.getEntry(body.get()).first;

        if(!_attachToBody(b, BodyAttachObjectType::OBJECT_TYPE_ENTITY)) return false;

        mEntitiesInWorld[b] = e;

        return true;
    }

    void DynamicsWorld::detatchEntityFromBody(DynamicsWorld::RigidBodyPtr body){
        btRigidBody* b = mBodyData.getEntry(body.get()).first;

        mEntitiesInWorld.erase(b);

        _detatchFromBody(b);
    }

    DynamicsWorld::BodyAttachObjectType DynamicsWorld::getBodyBindType(DynamicsWorld::RigidBodyPtr body){
        btRigidBody* b = mBodyData.getEntry(body.get()).first;

        return (DynamicsWorld::BodyAttachObjectType) b->getUserIndex();
    }

    void DynamicsWorld::setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic){
        //TODO this mutex has to be duplicated each time I want to do something with the thread. I don't like that.
        std::unique_lock<std::mutex> (dynWorldMutex);

        mDynLogic = dynLogic;
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

        btRigidBody* b = mBodyData.getEntry(body.get()).first;
        if(mBodiesInWorld.find(b) != mBodiesInWorld.end()) return;

        mBodiesInWorld.insert(b);

        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->inputBufferMutex);


        //Do a search for any entries in the buffer with the same pointer and invalidate them.
        _resetBufferEntries(b);
        mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_ADD, b});
    }

    bool DynamicsWorld::bodyInWorld(DynamicsWorld::RigidBodyPtr body){
        btRigidBody* b = mBodyData.getEntry(body.get()).first;

        return mBodiesInWorld.find(b) != mBodiesInWorld.end();
    }

    void DynamicsWorld::removeBody(DynamicsWorld::RigidBodyPtr body){
        std::unique_lock<std::mutex> dynamicWorldLock(dynWorldMutex);
        if(!mDynLogic) return;

        btRigidBody* b = mBodyData.getEntry(body.get()).first;
        if(mBodiesInWorld.find(b) == mBodiesInWorld.end()) return;

        mBodiesInWorld.erase(b);

        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->inputBufferMutex);

        _resetBufferEntries(b);
        mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_REMOVE, b});
    }
}
