#include "PhysicsBodyConstructor.h"

#include "btBulletDynamicsCommon.h"

#include "World/Physics/Worlds/DynamicsWorldMotionState.h"
#include "World/Physics/Worlds/DynamicsWorld.h"

#include "World/Slot/Recipe/PhysicsBodyRecipeData.h"

namespace AV{
    PhysicsBodyConstructor* PhysicsBodyConstructor::_bodyConstructor = 0;
    const PhysicsBodyConstructor::PhysicsChunkEntry PhysicsBodyConstructor::EMPTY_CHUNK_ENTRY = PhysicsBodyConstructor::PhysicsChunkEntry(0, 0);

    PhysicsBodyConstructor::PhysicsBodyConstructor(){

        _bodyConstructor = this;

        //Give the dynamics world a pointer to the body data.
        //The management of the bodies and their access is very much something the dynamics world needs to do.
        //Therefore, some sort of direct access to this data structure is important.
        DynamicsWorld::mBodyData = &mBodyData;
    }

    PhysicsBodyConstructor::~PhysicsBodyConstructor(){
        _bodyConstructor = 0;
    }

    PhysicsBodyConstructor::RigidBodyPtr PhysicsBodyConstructor::createRigidBody(btRigidBody::btRigidBodyConstructionInfo& info, PhysicsShapeManager::ShapePtr shape){
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
        void* val = mBodyData.storeEntry(RigidBodyEntry(bdy, shape));

        //Here val isn't actually a valid pointer, so the custom deleter doesn't need to delete anything.
        //Really this is just piggy-backing on the reference counting done by the shared pointers.
        RigidBodyPtr sharedPtr = RigidBodyPtr(val, _destroyRigidBody);


        return sharedPtr;
    }

    PhysicsBodyConstructor::PhysicsChunkEntry PhysicsBodyConstructor::createPhysicsChunk(const std::vector<PhysicsBodyRecipeData>& physicsBodyData, const std::vector<PhysicsShapeRecipeData>& physicsShapeData){
        std::vector<PhysicsShapeManager::ShapePtr> *shapeVector = new std::vector<PhysicsShapeManager::ShapePtr>();
        std::vector<btRigidBody*> *bodyVector = new std::vector<btRigidBody*>();

        //Creating physics shapes
        for(const PhysicsShapeRecipeData& data : physicsShapeData){
            int physicsShapeType;
            btVector3 scale;

            PhysicsShapeManager::ShapePtr shape = 0;
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

    PhysicsShapeManager::ShapePtr PhysicsBodyConstructor::getBodyShape(void* body){
        RigidBodyEntry& entry = mBodyData.getEntry(body);

        return entry.second;
    }

    void PhysicsBodyConstructor::_destroyRigidBody(void* body){
        //TODO maybe think of a different way to do this than using a static pointer.
        if(!_bodyConstructor) return;

        RigidBodyEntry& entry = _bodyConstructor->mBodyData.getEntry(body);

        DynamicsWorld::_destroyBody(entry.first);

        //For the shape it actually needs to be destroyed manually.
        entry.second.reset();

        _bodyConstructor->mBodyData.removeEntry(body);

    }
}
