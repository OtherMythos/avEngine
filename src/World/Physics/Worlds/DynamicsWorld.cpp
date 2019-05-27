#include "DynamicsWorld.h"

#include "btBulletDynamicsCommon.h"
#include "Threading/Thread/Physics/DynamicsWorldThreadLogic.h"

#include "Logger/Log.h"

namespace AV{
    DynamicsWorld::DynamicsWorld(){
        {
            //create a dynamic rigidbody

            //btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
            btCollisionShape* colShape = new btSphereShape(btScalar(1.));

            /// Create Dynamic Objects
            btTransform startTransform;
            startTransform.setIdentity();

            btScalar mass(1.f);

            //rigidbody is dynamic if and only if mass is non zero, otherwise static
            bool isDynamic = (mass != 0.f);

            btVector3 localInertia(0, 0, 0);
            if (isDynamic)
                colShape->calculateLocalInertia(mass, localInertia);

            startTransform.setOrigin(btVector3(2, 10, 0));

            //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
            btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
            btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
            tmpBody = new btRigidBody(rbInfo);

            //mDynamicsWorld->addRigidBody(body);
        }
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
    
    void DynamicsWorld::setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic){
        //TODO this mutex has to be duplicated each time I want to do something with the thread. I don't like that.
        std::unique_lock<std::mutex>(dynWorldMutex);
        
        mDynLogic = dynLogic;
        //TODO get rid of this when able.
        addBody(tmpBody);
    }
    
    void DynamicsWorld::addBody(btRigidBody* body){
        std::unique_lock<std::mutex> dynamicWorldLock(dynWorldMutex);
        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->inputBufferMutex);
        if(!mDynLogic) return;
        
        mDynLogic->inputBuffer.push_back({body});
    }
}
