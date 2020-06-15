#include "CollisionWorld.h"

#include "Threading/Thread/Physics/CollisionWorldThreadLogic.h"

namespace AV{
    ScriptDataPacker<PhysicsTypes::CollisionObjectEntry>* CollisionWorld::mCollisionObjectData;

    CollisionWorld::CollisionWorld(CollisionWorldId id)
        : mWorldId(id) {

    }

    CollisionWorld::~CollisionWorld(){

    }

    void CollisionWorld::update(){

    }

    void CollisionWorld::notifyOriginShift(Ogre::Vector3 offset){

    }

    void CollisionWorld::setCollisionWorldThreadLogic(CollisionWorldThreadLogic* threadLogic){
        mThreadLogic = threadLogic;
    }

    void CollisionWorld::addSender(PhysicsTypes::CollisionSenderPtr sender){
        if(!mThreadLogic) return;

        btCollisionObject* b = mCollisionObjectData->getEntry(sender.get()).first;

        //<Check if the body is in the world here>

        std::unique_lock<std::mutex> inputBufferLock(mThreadLogic->objectInputBufferMutex);

        //<Reset the current buffer entries>
        mThreadLogic->inputObjectCommandBuffer.push_back({CollisionWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_ADD_OBJECT, b});

        // btRigidBody* b = mBodyData->getEntry(body.get()).first;
        // if(_bodyInWorld(b)) return;
        //
        // mBodiesInWorld.insert(b);
        //
        // std::unique_lock<std::mutex> inputBufferLock(mThreadLogic->objectInputBufferMutex);
        //
        //
        // //Do a search for any entries in the buffer with the same pointer and invalidate them.
        // _resetBufferEntries(b);
        // mThreadLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_ADD_BODY, b});
    }
}
