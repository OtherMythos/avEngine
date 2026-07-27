#include "PhysicsBodyConstructor.h"

#include "btBulletDynamicsCommon.h"

#include "Physics/Worlds/DynamicsWorldMotionState.h"
#include "Physics/Worlds/DynamicsWorld.h"
#include "Physics/Worlds/CollisionWorld.h"
#include "PhysicsBodyDestructor.h"
#include "PhysicsShapeManager.h"
#include "PhysicsMetaDataManager.h"

#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include "Physics/PhysicsCollisionDataManager.h"
#include "System/BaseSingleton.h"
#include "Scripting/ScriptManager.h"
#include <stack>

#ifdef DEBUGGING_TOOLS
        #include "Developer/MeshVisualiser.h"
#endif

namespace AV{
    DataPacker<PhysicsTypes::RigidBodyEntry> PhysicsBodyConstructor::mBodyData;
    DataPacker<PhysicsTypes::CollisionObjectEntry> PhysicsBodyConstructor::mCollisionData;

    void PhysicsBodyConstructor::setup(){
        //Give the dynamics world a pointer to the body data.
        //The management of the bodies and their access is very much something the dynamics world needs to do.
        //Therefore, some sort of direct access to this data structure is important.
        DynamicsWorld::mBodyData = &mBodyData;
        CollisionWorld::mCollisionObjectData = &mCollisionData;

        PhysicsMetaDataManager::setup();
    }

    void PhysicsBodyConstructor::shutdown(){
        mBodyData.clear();
        mCollisionData.clear();
        PhysicsMetaDataManager::shutdown();
    }

    btCollisionObject* PhysicsBodyConstructor::_createCollisionObject(PhysicsTypes::ShapePtr shape, CollisionPackedInt data, void* dataId, const btVector3& origin, const btQuaternion& orientation){
        btCollisionObject *object = new btCollisionObject();
        object->setCollisionShape(shape.get());
        object->getWorldTransform().setOrigin(origin);
        object->getWorldTransform().setRotation(orientation);
        object->setUserIndex(data);
        object->setUserPointer(dataId);
        _setShapeAttached(shape.get());

        int setInternalId = PhysicsMetaDataManager::createDataForObject();
        object->setUserIndex3(setInternalId);

        return object;
    }

    PhysicsTypes::CollisionObjectPtr PhysicsBodyConstructor::createCollisionObject(PhysicsTypes::ShapePtr shape, CollisionPackedInt data, void* dataId, btVector3 origin, btQuaternion orientation){
        btCollisionObject* object = _createCollisionObject(shape, data, dataId, origin, orientation);

        void* val = mCollisionData.storeEntry({object, shape});

        PhysicsTypes::CollisionObjectPtr sharedPtr = PhysicsTypes::CollisionObjectPtr(val, _destroyCollisionObject);

        return sharedPtr;
    }

    PhysicsTypes::RigidBodyPtr PhysicsBodyConstructor::createRigidBody(btRigidBody::btRigidBodyConstructionInfo& info, PhysicsTypes::ShapePtr shape){
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

        _setShapeAttached(info.m_collisionShape);

        int setInternalId = PhysicsMetaDataManager::createDataForObject();
        bdy->setUserIndex3(setInternalId);

        //We store a copy of the pointer to the shape as well.
        //That way there's no chance of the shape being destroyed while the rigid body is still using it.
        void* val = mBodyData.storeEntry({bdy, shape});

        //Here val isn't actually a valid pointer, so the custom deleter doesn't need to delete anything.
        //Really this is just piggy-backing on the reference counting done by the shared pointers.
        PhysicsTypes::RigidBodyPtr sharedPtr = PhysicsTypes::RigidBodyPtr(val, _destroyRigidBody);


        return sharedPtr;
    }

    btRigidBody* PhysicsBodyConstructor::createTerrainBody(btHeightfieldTerrainShape* terrainShape){
        btRigidBody::btRigidBodyConstructionInfo info(0, 0, terrainShape);

        info.m_startWorldTransform.setIdentity();
        //The terrain is positioned upon insertion into the physics world.

        return new btRigidBody(info);
    }

    void PhysicsBodyConstructor::_setShapeAttached(btCollisionShape* shape){
        void* start = shape->getUserPointer();

        //Set the final bit of the first 32 bits to be a 1. This represents being attached.
        uintptr_t newVal = (uintptr_t)start | 0x80000000;
        shape->setUserPointer((void*)newVal);
    }

    PhysicsTypes::ShapePtr PhysicsBodyConstructor::getBodyShape(void* body){
        PhysicsTypes::RigidBodyEntry& entry = mBodyData.getEntry(body);

        return entry.second;
    }

    void PhysicsBodyConstructor::_destroyRigidBody(void* body){
        PhysicsTypes::RigidBodyEntry& entry = mBodyData.getEntry(body);

        //Just remove the body here. Don't actually destroy it.
        DynamicsWorld::_removeBody(entry.first);

        //Shape destruction needs to happen first to ensure the correct queuing of objects.

        //For the shape it actually needs to be destroyed manually.
        entry.second.reset();

        mBodyData.removeEntry(body);

        PhysicsBodyDestructor::destroyRigidBody(entry.first);
    }

    void PhysicsBodyConstructor::_destroyCollisionObject(void* object){
        PhysicsTypes::CollisionObjectEntry& entry = mCollisionData.getEntry(object);

        CollisionWorld::_removeObject(entry.first);

        entry.second.reset();
        mCollisionData.removeEntry(object);

        PhysicsBodyDestructor::destroyCollisionObject(entry.first);
    }
}
