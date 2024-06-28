#include "DynamicsWorld.h"

#include "Threading/Thread/Physics/DynamicsWorldThreadLogic.h"
#include "World/Physics/Worlds/DynamicsWorldMotionState.h"
#include "World/Physics/PhysicsMetaDataManager.h"

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
        //As for the shift, we need to essentually discard whatever's in the buffer, as a shift has just happened and it's all stale.
        if(mDynLogic->outputBuffer.size() > 0 && !physicsPaused && !mShiftPerformedLastFrame){
            for(const DynamicsWorldThreadLogic::OutputBufferEntry& entry : mDynLogic->outputBuffer){
                BodyAttachObjectType type = (BodyAttachObjectType) entry.body->getUserIndex();

                //A potential optimisation here would be making it so that these entries don't get pushed in the first place.
                //However that would mean I would have to read the user index on the physics thread side, which at the moment isn't possible.
                if(type == BodyAttachObjectType::OBJECT_TYPE_NONE) continue;

                //There is a chance the buffer contains information about a body that was recently removed from the world. We don't want that.
                //This is a potential optimisation. Rather than searching all entities in the world, just search ones that were removed that frame.
                if(!_bodyInWorld(entry.body)) continue;
                //There is also a chance the output buffer contains stale data, for instance if a body has recently been moved.
                //After a move, the body is added to this list to not be processed this frame.
                if(_shouldIgnoreBody(entry.body)) continue;

                DynamicsObjectWorldData& copyData = mObjectWorldData[entry.body];
                copyData.position = entry.pos;
                copyData.orientation = entry.orientation;
                copyData.linearVelocity = entry.linearVelocity;

                switch(type){
                    case BodyAttachObjectType::OBJECT_TYPE_ENTITY: {
                        //TODO there's a chance this might return something invalid. Check that.
                        eId entity = mEntitiesInWorld[entry.body];
                        mEntityTransformData.push_back({entity, entry.pos, entry.orientation});
                        break;
                    }
                    case BodyAttachObjectType::OBJECT_TYPE_MESH: {
                        auto it = mMeshesInWorld.find(entry.body);
                        if(it == mMeshesInWorld.end()) continue; //If that mesh no longer exists in the world for whatever reason.

                        Ogre::SceneNode* node = (*it).second;
                        mMeshTransformData.push_back({node, entry.pos, entry.orientation});
                        break;
                    }
                    default:{
                        break;
                    }
                };
            }
        }
        //We've read the values, and don't want to risk reading them again, so the buffer should be cleared.
        mDynLogic->outputBuffer.clear();
        mIgnoredBodies.clear();

        //We no longer need the lock.
        outputBufferLock.unlock();

        mShiftPerformedLastFrame = false;

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

    void DynamicsWorld::notifyOriginShift(const Ogre::Vector3& offset, const SlotPosition& newPos){
        if(!mDynLogic) return;
        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->objectInputBufferMutex);

        btVector3 orig(offset.x, offset.y, offset.z);
        mDynLogic->worldOriginChangeOffset = orig;
        mDynLogic->worldOriginChangeNewPosition = newPos;
        mDynLogic->worldShifted = true;

        mShiftPerformedLastFrame = true;
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

        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->objectInputBufferMutex);


        //Do a search for any entries in the buffer with the same pointer and invalidate them.
        _resetBufferEntries(b);
        mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_ADD_BODY, b, 0, 0});
    }

    void DynamicsWorld::addTerrainBody(btRigidBody* terrain, int x, int y){
        if(!mDynLogic) return;

        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->objectInputBufferMutex);
        _resetBufferEntries(terrain);
        mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_ADD_TERRAIN, terrain, x, y});
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

        _resetBufferEntries(b);
        mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_REMOVE_BODY, b, 0, 0});
    }

    bool DynamicsWorld::_bodyInWorld(btRigidBody* bdy) const{
        return mBodiesInWorld.find(bdy) != mBodiesInWorld.end();
    }

    bool DynamicsWorld::_shouldIgnoreBody(btRigidBody* bdy){
        return mIgnoredBodies.find(bdy) != mIgnoredBodies.end();
    }

    uint32 DynamicsWorld::_findPhysicsChunksHole(){
        auto it = mPhysicsChunksInWorld.begin();
        uint32 index = 0;
        while(it != mPhysicsChunksInWorld.end()){
            //0 represents a null value, so the returned result needs to be shifted by 1.
            if(*it == PhysicsTypes::EMPTY_CHUNK_ENTRY) return index + 1;
            index++;
            it++;
        }

        return 0;
    }

    uint32 DynamicsWorld::addPhysicsChunk(const PhysicsTypes::PhysicsChunkEntry& chunk){
        //If this physics chunk already exists in the world don't do anything.
        auto it = std::find(mPhysicsChunksInWorld.begin(), mPhysicsChunksInWorld.end(), chunk);
        if(it != mPhysicsChunksInWorld.end()) {
            //Return the index of the found value.
            return static_cast<uint32>(it - mPhysicsChunksInWorld.begin());
        }

        uint32 targetIndex = _findPhysicsChunksHole();
        if(targetIndex == 0){
            //No hole was found to insert into.
            mPhysicsChunksInWorld.push_back(chunk);
            targetIndex = static_cast<uint32>(mPhysicsChunksInWorld.size()) - 1;
        }else{
            targetIndex -= 1;
            mPhysicsChunksInWorld[targetIndex] = chunk;
        }

        //Cast the vector pointer to a rigid body pointer.
        //This is just so the relevant functions will accept it.
        //We id the chunk by the pointer to the body vector.
        btRigidBody* vectorBodyEntries = reinterpret_cast<btRigidBody*>(chunk.second);

        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->objectInputBufferMutex);
        //Remove add and destroy for this chunk.
        _resetBufferEntries(vectorBodyEntries);
        mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_ADD_CHUNK, vectorBodyEntries, chunk.slotX, chunk.slotY});

        return targetIndex;
    }

    void DynamicsWorld::removePhysicsChunk(uint32 chunkId, bool requestWorldRemoval){
        assert(chunkId < mPhysicsChunksInWorld.size());

        btRigidBody* vectorBodyEntries = reinterpret_cast<btRigidBody*>(mPhysicsChunksInWorld[chunkId].second);

        if(requestWorldRemoval){
            std::unique_lock<std::mutex> inputBufferLock(mDynLogic->objectInputBufferMutex);

            mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_REMOVE_CHUNK, vectorBodyEntries, 0, 0});
        }

        //We still need to check the buffer to make sure it doesn't contain any stale entries (add commands after a recent deletion).
        _resetBufferEntries(vectorBodyEntries);

        //Turn the entry in the vector into a hole.
        mPhysicsChunksInWorld[chunkId] = PhysicsTypes::EMPTY_CHUNK_ENTRY;
    }

    void DynamicsWorld::setBodyPosition(PhysicsTypes::RigidBodyPtr body, btVector3 pos){
        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->inputBufferMutex);

        btRigidBody* b = mBodyData->getEntry(body.get()).first;
        mDynLogic->inputBuffer.push_back({DynamicsWorldThreadLogic::InputBufferCommandType::COMMAND_TYPE_SET_POSITION, b, pos});

        mIgnoredBodies.insert(b);
    }

    void DynamicsWorld::setBodyLinearFactor(PhysicsTypes::RigidBodyPtr body, btVector3 factor){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;

        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->inputBufferMutex);
        mDynLogic->inputBuffer.push_back({DynamicsWorldThreadLogic::InputBufferCommandType::COMMAND_TYPE_SET_LINEAR_FACTOR, b, factor});

        //mIgnoredBodies.insert(b);
    }

    void DynamicsWorld::setBodyLinearVelocity(PhysicsTypes::RigidBodyPtr body, btVector3 velocity){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;
        mObjectWorldData[b].linearVelocity = velocity;

        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->inputBufferMutex);
        mDynLogic->inputBuffer.push_back({DynamicsWorldThreadLogic::InputBufferCommandType::COMMAND_TYPE_SET_LINEAR_VELOCITY, b, velocity});

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
