#include "DynamicsWorld.h"

#include "Threading/Thread/Physics/DynamicsWorldThreadLogic.h"
#include "World/Physics/Worlds/DynamicsWorldMotionState.h"

#include "Ogre.h"

#include "Logger/Log.h"

namespace AV{
    DynamicsWorldThreadLogic* DynamicsWorldMotionState::dynLogic = 0;

    ScriptDataPacker<PhysicsBodyConstructor::RigidBodyEntry>* DynamicsWorld::mBodyData;
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

        //Check if there's anything in the command buffer that needs addressing.
        //This isn't guaranteed to contain anything, for example if the current physics processing is taking a long time.
        if(mDynLogic->outputBuffer.size() > 0){
            for(const DynamicsWorldThreadLogic::outputBufferEntry& entry : mDynLogic->outputBuffer){
                BodyAttachObjectType type = (BodyAttachObjectType) entry.body->getUserIndex();

                //A potential optimisation here would be making it so that these entries don't get pushed in the first place.
                //However that would mean I would have to read the user index on the physics thread side, which at the moment isn't possible.
                if(type == BodyAttachObjectType::OBJECT_TYPE_NONE) continue;

                //There is a chance the buffer contains information about a body that was recently removed from the world. We don't want that.
                //This is a potential optimisation. Rather than searching all entities in the world, just search ones that were removed that frame.
                if(!_bodyInWorld(entry.body)) continue;

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

        //We no longer need the lock.
        outputBufferLock.unlock();

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

    bool DynamicsWorld::attachEntityToBody(PhysicsBodyConstructor::RigidBodyPtr body, eId e){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;

        if(!_attachToBody(b, BodyAttachObjectType::OBJECT_TYPE_ENTITY)) return false;

        mEntitiesInWorld[b] = e;

        return true;
    }

    void DynamicsWorld::detatchEntityFromBody(PhysicsBodyConstructor::RigidBodyPtr body){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;

        mEntitiesInWorld.erase(b);

        _detatchFromBody(b);
    }

    bool DynamicsWorld::attachMeshToBody(PhysicsBodyConstructor::RigidBodyPtr body, Ogre::SceneNode* meshNode){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;

        if(!_attachToBody(b, BodyAttachObjectType::OBJECT_TYPE_MESH)) return false;

        mMeshesInWorld[b] = meshNode;

        return true;
    }

    void DynamicsWorld::detachMeshFromBody(PhysicsBodyConstructor::RigidBodyPtr body){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;

        mMeshesInWorld.erase(b);

        _detatchFromBody(b);
    }

    DynamicsWorld::BodyAttachObjectType DynamicsWorld::getBodyBindType(PhysicsBodyConstructor::RigidBodyPtr body){
        btRigidBody* b = mBodyData->getEntry(body.get()).first;

        return (DynamicsWorld::BodyAttachObjectType) b->getUserIndex();
    }

    void DynamicsWorld::setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic){
        mDynLogic = dynLogic;
    }

    void DynamicsWorld::_resetBufferEntries(btRigidBody* b){
        for(DynamicsWorldThreadLogic::objectCommandBufferEntry& e : mDynLogic->inputObjectCommandBuffer){
            if(e.type == DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_NONE
                || e.body != b) continue;

            //Here we can assume a match was found.
            //I just invalidate the entry rather than deleting it, as it's going to be more efficient than shifting the vector.
            e.type = DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_NONE;
            e.body = 0;
        }
    }

    void DynamicsWorld::addBody(PhysicsBodyConstructor::RigidBodyPtr body){
        if(!mDynLogic) return;

        btRigidBody* b = mBodyData->getEntry(body.get()).first;
        if(_bodyInWorld(b)) return;

        mBodiesInWorld.insert(b);

        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->inputBufferMutex);


        //Do a search for any entries in the buffer with the same pointer and invalidate them.
        _resetBufferEntries(b);
        mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_ADD, b});
    }

    bool DynamicsWorld::bodyInWorld(PhysicsBodyConstructor::RigidBodyPtr body) const{
        btRigidBody* b = mBodyData->getEntry(body.get()).first;

        return _bodyInWorld(b);
    }

    void DynamicsWorld::removeBody(PhysicsBodyConstructor::RigidBodyPtr body){
        if(!mDynLogic) return;

        btRigidBody* b = mBodyData->getEntry(body.get()).first;
        if(!_bodyInWorld(b)) return;

        mBodiesInWorld.erase(b);

        std::unique_lock<std::mutex> inputBufferLock(mDynLogic->inputBufferMutex);

        _resetBufferEntries(b);
        mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_REMOVE, b});
    }

    bool DynamicsWorld::_bodyInWorld(btRigidBody* bdy) const{
        return mBodiesInWorld.find(bdy) != mBodiesInWorld.end();
    }

    void DynamicsWorld::_deleteBodyPtr(btRigidBody* bdy){
        DynamicsWorldMotionState* motionState = (DynamicsWorldMotionState*)bdy->getMotionState();
        if(motionState){
            delete motionState;
        }
        delete bdy;
    }

    void DynamicsWorld::_destroyBodyInternal(btRigidBody* bdy){
        if(!mDynLogic){
            //There is no world altogether. In this case the body should just be deleted.
            _deleteBodyPtr(bdy);

            return;
        }else{
            mBodiesInWorld.erase(bdy);

            //There is a chance the object might already be in the dynamics world, or in the input buffer for insertion.
            std::unique_lock<std::mutex> inputBufferLock(mDynLogic->inputBufferMutex);

            _resetBufferEntries(bdy);
            mDynLogic->inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_DESTROY, bdy});
        }

    }

    void DynamicsWorld::_destroyBody(btRigidBody* bdy){
        if(!dynWorld){
            //If there is no instance of the dynamics world just delete the body.
            dynWorld->_deleteBodyPtr(bdy);
            return;
        }

        dynWorld->_destroyBodyInternal(bdy);
    }
}
