#include "PhysicsBodyConstructor.h"

#include "btBulletDynamicsCommon.h"

#include "World/Physics/Worlds/DynamicsWorldMotionState.h"
#include "World/Physics/Worlds/DynamicsWorld.h"
#include "PhysicsBodyDestructor.h"
#include "PhysicsShapeManager.h"

#include "World/Slot/Recipe/PhysicsBodyRecipeData.h"

namespace AV{
    ScriptDataPacker<PhysicsBodyConstructor::RigidBodyEntry> PhysicsBodyConstructor::mBodyData;

    void PhysicsBodyConstructor::setup(){
        //Give the dynamics world a pointer to the body data.
        //The management of the bodies and their access is very much something the dynamics world needs to do.
        //Therefore, some sort of direct access to this data structure is important.
        DynamicsWorld::mBodyData = &mBodyData;
    }

    void PhysicsBodyConstructor::shutdown(){
        mBodyData.clear();
    }

    PhysicsBodyConstructor::RigidBodyPtr PhysicsBodyConstructor::createRigidBody(btRigidBody::btRigidBodyConstructionInfo& info, PhysicsTypes::ShapePtr shape){
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

        //We store a copy of the pointer to the shape as well.
        //That way there's no chance of the shape being destroyed while the rigid body is still using it.
        void* val = mBodyData.storeEntry(RigidBodyEntry(bdy, shape));

        //Here val isn't actually a valid pointer, so the custom deleter doesn't need to delete anything.
        //Really this is just piggy-backing on the reference counting done by the shared pointers.
        RigidBodyPtr sharedPtr = RigidBodyPtr(val, _destroyRigidBody);


        return sharedPtr;
    }

    void PhysicsBodyConstructor::_setShapeAttached(btCollisionShape* shape){
        void* start = shape->getUserPointer();

        //Set the final bit of the first 32 bits to be a 1. This represents being attached.
        uintptr_t newVal = (uintptr_t)start | 0x80000000;
        shape->setUserPointer((void*)newVal);
    }

    PhysicsTypes::PhysicsChunkEntry PhysicsBodyConstructor::createPhysicsChunk(const std::vector<PhysicsBodyRecipeData>& physicsBodyData, const std::vector<PhysicsShapeRecipeData>& physicsShapeData){
        std::vector<PhysicsTypes::ShapePtr> *shapeVector = new std::vector<PhysicsTypes::ShapePtr>();
        std::vector<btRigidBody*> *bodyVector = new std::vector<btRigidBody*>();

        //Creating physics shapes
        for(const PhysicsShapeRecipeData& data : physicsShapeData){
            int physicsShapeType;
            btVector3 scale;

            PhysicsTypes::ShapePtr shape = 0;
            PhysicsShapeManager::PhysicsShapeType shapeType = static_cast<PhysicsShapeManager::PhysicsShapeType>(data.physicsShapeType);
            switch(shapeType){
                case PhysicsShapeManager::PhysicsShapeType::CubeShape:{
                    shape = PhysicsShapeManager::getBoxShape(data.scale);
                    break;
                }
                case PhysicsShapeManager::PhysicsShapeType::SphereShape:{
                    shape = PhysicsShapeManager::getSphereShape(data.scale.x());
                    break;
                }
                case PhysicsShapeManager::PhysicsShapeType::CapsuleShape:{
                    shape = PhysicsShapeManager::getCapsuleShape(data.scale.x(), data.scale.y());
                    break;
                }
                default:{
                    //We shouldn't reach this point.
                    assert(false);
                }
            };

            shapeVector->push_back(shape);
        }

        //mass, motionstate, collision shape
        btRigidBody::btRigidBodyConstructionInfo bodyInfo(0, 0, 0);
        bodyInfo.m_startWorldTransform.setIdentity();
        //Always static, so 0.
        for(const PhysicsBodyRecipeData& data : physicsBodyData){
            bodyInfo.m_collisionShape = (*shapeVector)[data.shapeId].get();

            btTransform transform;
            transform.setOrigin(data.pos);
            transform.setRotation(data.orientation);

            btRigidBody *bdy = new btRigidBody(bodyInfo);

            bodyVector->push_back(bdy);
        }

        return {shapeVector, bodyVector};
    }

    PhysicsTypes::ShapePtr PhysicsBodyConstructor::getBodyShape(void* body){
        RigidBodyEntry& entry = mBodyData.getEntry(body);

        return entry.second;
    }

    void PhysicsBodyConstructor::_destroyRigidBody(void* body){
        RigidBodyEntry& entry = mBodyData.getEntry(body);

        //Just remove the body here. Don't actually destroy it.
        DynamicsWorld::_removeBody(entry.first);

        //Shape destruction needs to happen first to ensure the correct queuing of objects.

        //For the shape it actually needs to be destroyed manually.
        entry.second.reset();

        mBodyData.removeEntry(body);

        PhysicsBodyDestructor::destroyRigidBody(entry.first);
    }
}
