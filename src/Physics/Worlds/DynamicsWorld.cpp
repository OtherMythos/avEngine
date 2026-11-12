#include "DynamicsWorld.h"

#include "Threading/Thread/Physics/DynamicsWorldThreadLogic.h"
#include "Physics/Worlds/DynamicsWorldMotionState.h"
#include "Physics/PhysicsMetaDataManager.h"

#include "Ogre.h"

#include "Logger/Log.h"
#include <iostream>
#include <map>

#include "System/Pause/PauseState.h"
#include "System/EngineFlags.h"

namespace AV{
    DynamicsWorldThreadLogic* DynamicsWorldMotionState::dynLogic = 0;

    DataPacker<PhysicsTypes::RigidBodyEntry>* DynamicsWorld::mBodyData;
    DynamicsWorld* DynamicsWorld::dynWorld = 0;

    DynamicsWorld::DynamicsWorld(){
        dynWorld = this;
    }

    DynamicsWorld::~DynamicsWorld(){
        dynWorld = 0;
    }

    void DynamicsWorld::update(){
        std::unique_lock<std::mutex> outputBufferLock(mDynLogic->outputBufferMutex);

        mEntityTransformData.clear();

        bool physicsPaused = (PauseState::getMask() & PAUSE_TYPE_PHYSICS) > 0;

        //Check if there's anything in the command buffer that needs addressing.
        //This isn't guaranteed to contain anything, for example if the current physics processing is taking a long time.
        const btVector3 outputDelta = mOrigin - mDynLogic->outputOrigin;
        if(mDynLogic->outputBuffer.size() > 0 && !physicsPaused){
            for(const DynamicsWorldThreadLogic::OutputBufferEntry& entry : mDynLogic->outputBuffer){

                //There is a chance the buffer contains information about a body that was recently removed from the world. We don't want that.
                //This is a potential optimisation. Rather than searching all entities in the world, just search ones that were removed that frame.
                if(!_bodyInWorld(entry.body)) continue;
                //There is also a chance the output buffer contains stale data, for instance if a body has recently been moved.
                //After a move, the body is added to this list to not be processed this frame.
                if(_shouldIgnoreBody(entry.body)) continue;

                BodyAttachObjectType type = (BodyAttachObjectType) entry.body->getUserIndex();
                const btVector3 position = entry.pos - outputDelta;

                //Recorded for every body, attached or not, so a body owned purely by script can still be asked where it is.
                DynamicsObjectWorldData& copyData = mObjectWorldData[entry.body];
                copyData.position = position;
                copyData.orientation = entry.orientation;
                copyData.linearVelocity = entry.linearVelocity;

                if(type == BodyAttachObjectType::OBJECT_TYPE_NONE) continue;

                switch(type){
                    case BodyAttachObjectType::OBJECT_TYPE_ENTITY: {
                        //TODO there's a chance this might return something invalid. Check that.
                        eId entity = mEntitiesInWorld[entry.body];
                        mEntityTransformData.push_back({entity, position, entry.orientation});
                        break;
                    }
                    case BodyAttachObjectType::OBJECT_TYPE_MESH: {
                        auto it = mMeshesInWorld.find(entry.body);
                        if(it == mMeshesInWorld.end()) continue; //If that mesh no longer exists in the world for whatever reason.

                        Ogre::SceneNode* node = (*it).second;
                        mMeshTransformData.push_back({node, position, entry.orientation});
                        break;
                    }
                    default:{
                        break;
                    }
                };
            }
        }
        //Not part of the above, as that only runs if something moved in the latest step, and these build up across steps.
        mImpacts.clear();
        if(!physicsPaused){
            for(const DynamicsWorldThreadLogic::OutputImpactEntry& entry : mDynLogic->outputImpactBuffer){
                //Checked here rather than on the physics thread, as a body is only safe to read once it's known to be in the world.
                if(!_bodyInWorld(entry.bodyA) || !_bodyInWorld(entry.bodyB)) continue;

                mImpacts.push_back({
                    PhysicsMetaDataManager::getObjectMeta(entry.bodyA->getUserIndex3()).id,
                    PhysicsMetaDataManager::getObjectMeta(entry.bodyB->getUserIndex3()).id,
                    entry.pos - outputDelta,
                    entry.strength
                });
            }
        }
        mDynLogic->outputImpactBuffer.clear();

        //We've read the values, and don't want to risk reading them again, so the buffer should be cleared.
        mDynLogic->outputBuffer.clear();
        mIgnoredBodies.clear();

        //We no longer need the lock.
        outputBufferLock.unlock();

        assert(!EngineFlags::sceneClean());
        if(mMeshTransformData.size() > 0){
            for(const MeshTransformData& i : mMeshTransformData){
                //Currently I'm doing the repositioning in the dynamcis world.
                //Admittedly this isn't a physics thing but there was no where else sensible to put it.
                //TODO move it if a more sensible place is thought of.
                i.meshNode->setPosition(Ogre::Vector3(i.pos.x(), i.pos.y(), i.pos.z()));
                i.meshNode->setOrientation(Ogre::Quaternion(i.orientation.w(), i.orientation.x(), i.orientation.y(), i.orientation.z()));
            }
            mMeshTransformData.clear();
        }
    }

    btVector3 DynamicsWorld::getBodyPosition(PhysicsTypes::RigidBodyPtr body){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;

        auto it = mObjectWorldData.find(b);
        if(it == mObjectWorldData.end()) return btVector3(0, 0, 0);

        return (*it).second.position;
    }

    btQuaternion DynamicsWorld::getBodyOrientation(PhysicsTypes::RigidBodyPtr body){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;

        auto it = mObjectWorldData.find(b);
        if(it == mObjectWorldData.end()) return btQuaternion::getIdentity();

        return (*it).second.orientation;
    }

    btVector3 DynamicsWorld::getBodyLinearVelocity(PhysicsTypes::RigidBodyPtr body){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;

        auto it = mObjectWorldData.find(b);
        if(it == mObjectWorldData.end()) return btVector3(0, 0, 0);

        return (*it).second.linearVelocity;
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

    bool DynamicsWorld::attachEntityToBody(PhysicsTypes::RigidBodyPtr body, eId e){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;

        if(!_attachToBody(b, BodyAttachObjectType::OBJECT_TYPE_ENTITY)) return false;

        mEntitiesInWorld[b] = e;
        PhysicsMetaDataManager::setEntityForObject(b->getUserIndex3(), e);

        return true;
    }

    void DynamicsWorld::detatchEntityFromBody(PhysicsTypes::RigidBodyPtr body){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;

        mEntitiesInWorld.erase(b);
        PhysicsMetaDataManager::setEntityForObject(b->getUserIndex3(), eId::INVALID);

        _detatchFromBody(b);
    }

    bool DynamicsWorld::attachMeshToBody(PhysicsTypes::RigidBodyPtr body, Ogre::SceneNode* meshNode){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;

        if(!_attachToBody(b, BodyAttachObjectType::OBJECT_TYPE_MESH)) return false;

        mMeshesInWorld[b] = meshNode;

        return true;
    }

    void DynamicsWorld::detachMeshFromBody(PhysicsTypes::RigidBodyPtr body){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;

        mMeshesInWorld.erase(b);

        _detatchFromBody(b);
    }

    DynamicsWorld::BodyAttachObjectType DynamicsWorld::getBodyBindType(PhysicsTypes::RigidBodyPtr body){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;

        return (DynamicsWorld::BodyAttachObjectType) b->getUserIndex();
    }

    void DynamicsWorld::setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic){
        mDynLogic = dynLogic;
    }

    void DynamicsWorld::setImpactThreshold(float strength){
        if(!mDynLogic) return;

        mDynLogic->impactThreshold.store(strength);
    }

    void DynamicsWorld::shiftOrigin(const btVector3& delta){
        if(!mDynLogic) return;
        std::lock_guard<std::mutex> lock(mDynLogic->objectInputBufferMutex);
        mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_SHIFT_ORIGIN, nullptr, delta});
        mOrigin += delta;
        for(auto& entry : mObjectWorldData) entry.second.position -= delta;
        for(auto& entry : mEntityTransformData) entry.pos -= delta;
        for(auto& entry : mMeshTransformData) entry.pos -= delta;
        for(auto& entry : mImpacts) entry.pos -= delta;
    }

    DynamicsWorldThreadLogic::Statistics DynamicsWorld::getStatistics() const{
        if(!mDynLogic) return {};
        std::lock_guard<std::mutex> lock(mDynLogic->outputBufferMutex);
        return mDynLogic->statistics;
    }

    void DynamicsWorld::_resetBufferEntries(btRigidBody* b){
        for(DynamicsWorldThreadLogic::ObjectCommandBufferEntry& e : mDynLogic->inputObjectCommandBuffer){
            if(e.type == DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_NONE
                || e.body != b) continue;

            //Here we can assume a match was found.
            //I just invalidate the entry rather than deleting it, as it's going to be more efficient than shifting the vector.
            e.type = DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_NONE;
            e.body = 0;
        }
    }

    void DynamicsWorld::addBody(PhysicsTypes::RigidBodyPtr body){
        if(!mDynLogic) return;

        btRigidBody* b = mBodyData->getEntry(body.get()).first;
        if(_bodyInWorld(b)) return;

        mBodiesInWorld.insert(b);

        //Until the physics thread first reports the body, answer with where it was created. Safe to read here as the
        //thread only learns of the body through the command pushed below.
        DynamicsObjectWorldData& startData = mObjectWorldData[b];
        startData.position = b->getWorldTransform().getOrigin();
        startData.orientation = b->getWorldTransform().getRotation();
        startData.linearVelocity = b->getLinearVelocity();

        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->objectInputBufferMutex);


        mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_ADD_BODY, b});
    }

    bool DynamicsWorld::bodyInWorld(PhysicsTypes::RigidBodyPtr body) const{
        btRigidBody* b = mBodyData->getEntry(body.get()).first;

        return _bodyInWorld(b);
    }

    void DynamicsWorld::removeBody(PhysicsTypes::RigidBodyPtr body){
        if(!mDynLogic) return;

        btRigidBody* b = mBodyData->getEntry(body.get()).first;
        if(!_bodyInWorld(b)) return;

        mBodiesInWorld.erase(b);
        mObjectWorldData.erase(b);

        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->objectInputBufferMutex);

        mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_REMOVE_BODY, b});
    }

    bool DynamicsWorld::_bodyInWorld(btRigidBody* bdy) const{
        return mBodiesInWorld.find(bdy) != mBodiesInWorld.end();
    }

    bool DynamicsWorld::_shouldIgnoreBody(btRigidBody* bdy){
        return mIgnoredBodies.find(bdy) != mIgnoredBodies.end();
    }

    void DynamicsWorld::setBodyPosition(PhysicsTypes::RigidBodyPtr body, btVector3 pos){
        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->objectInputBufferMutex);

        btRigidBody* b = mBodyData->getEntry(body.get()).first;
        //As with the velocity, so asking where the body is straight after moving it answers with the move rather than
        //the last report, which this frame's output is about to be ignored in favour of.
        mObjectWorldData[b].position = pos;
        mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_SET_POSITION, b, pos});

        mIgnoredBodies.insert(b);
    }

    void DynamicsWorld::setBodyLinearFactor(PhysicsTypes::RigidBodyPtr body, btVector3 factor){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;

        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->objectInputBufferMutex);
        mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_SET_LINEAR_FACTOR, b, factor});

        //mIgnoredBodies.insert(b);
    }

    void DynamicsWorld::setBodyLinearVelocity(PhysicsTypes::RigidBodyPtr body, btVector3 velocity){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;
        mObjectWorldData[b].linearVelocity = velocity;

        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->objectInputBufferMutex);
        mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_SET_LINEAR_VELOCITY, b, velocity});

        //Certain functions don't need this. If called each frame it can lead to things not moving at all.
        //mIgnoredBodies.insert(b);
    }

    //This function is called by the shared pointer destruction. It is static so has to check for the dynamic world existence.
    void DynamicsWorld::_removeBody(btRigidBody* bdy){
        if(!dynWorld){
            return;
        }

        //The buffer still needs to be cleared.
        {
            std::unique_lock<std::mutex> inputBufferLock(dynWorld->mDynLogic->objectInputBufferMutex);
            dynWorld->_resetBufferEntries(bdy);
        }

        dynWorld->mBodiesInWorld.erase(bdy);
        dynWorld->mObjectWorldData.erase(bdy);
    }

    CollisionInternalId DynamicsWorld::getBodyInternalIdStatic(PhysicsTypes::RigidBodyPtr body){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;
        return PhysicsMetaDataManager::getObjectMeta(b->getUserIndex3()).id;
    }
}
