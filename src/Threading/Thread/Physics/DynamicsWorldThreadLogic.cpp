#include "DynamicsWorldThreadLogic.h"

#include "Logger/Log.h"
#include "btBulletDynamicsCommon.h"

#include "Physics/Worlds/DynamicsWorldMotionState.h"
#include "System/SystemSetup/SystemSettings.h"

#include "OgreVector3.h"
#include <chrono>

namespace AV{
    DynamicsWorldThreadLogic::DynamicsWorldThreadLogic()
        : PhysicsWorldThreadLogic() {
        DynamicsWorldMotionState::dynLogic = this;

        mWorldDestroyComplete = false;
    }

    void DynamicsWorldThreadLogic::updateWorld(float timeStep){
        const auto started = std::chrono::steady_clock::now();

        checkInputBuffers();

        btDiscreteDynamicsWorld* dynWorld = static_cast<btDiscreteDynamicsWorld*>(mPhysicsWorld);
        //maxSubSteps of 0 puts bullet in variable timestep mode, where it advances by exactly
        //timeStep once and leaves its own internal accumulator alone. PhysicsThread already
        //accumulates and only ever passes a constant 1/PhysicsUpdateRate, so letting bullet
        //accumulate as well would just stack a second, drifting accumulator underneath ours.
        dynWorld->stepSimulation(timeStep, 0);

        updateOutputBuffer();
        std::lock_guard<std::mutex> lock(outputBufferMutex);
        statistics.dynamicBodies = 0;
        statistics.staticBodies = 0;
        statistics.awakeBodies = 0;
        for(int i = 0; i < dynWorld->getNumCollisionObjects(); i++){
            const btRigidBody* body = btRigidBody::upcast(dynWorld->getCollisionObjectArray()[i]);
            if(body->isStaticOrKinematicObject()) statistics.staticBodies++;
            else{
                statistics.dynamicBodies++;
                if(body->isActive()) statistics.awakeBodies++;
            }
        }
        statistics.steps++;
        statistics.stepMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
    }

    void DynamicsWorldThreadLogic::checkInputBuffers(){
        _processObjectInputBuffer();
    }

    void DynamicsWorldThreadLogic::_processObjectInputBuffer(){
        std::unique_lock<std::mutex> inputBufferLock(objectInputBufferMutex);
        if(inputObjectCommandBuffer.size() <= 0) return;

        btDiscreteDynamicsWorld* dynamicsWorld = static_cast<btDiscreteDynamicsWorld*>(mPhysicsWorld);
        for(const ObjectCommandBufferEntry& entry : inputObjectCommandBuffer){
            if(entry.type == ObjectCommandType::COMMAND_TYPE_NONE) continue;

            btRigidBody* b = entry.body;
            switch(entry.type){
                case ObjectCommandType::COMMAND_TYPE_ADD_BODY:
                    if(!b->isInWorld()) dynamicsWorld->addRigidBody(b);
                    break;
                case ObjectCommandType::COMMAND_TYPE_REMOVE_BODY:
                    if(b->isInWorld()) dynamicsWorld->removeRigidBody(b);
                    break;
                case ObjectCommandType::COMMAND_TYPE_SHIFT_ORIGIN:
                    _shiftOrigin(entry.val);
                    break;
                case ObjectCommandType::COMMAND_TYPE_SET_POSITION:{
                    btTransform transform = b->getWorldTransform();
                    transform.setOrigin(entry.val);
                    b->setWorldTransform(transform);
                    b->setInterpolationWorldTransform(transform);
                    if(b->getMotionState()) b->getMotionState()->setWorldTransform(transform);
                    if(b->isInWorld()) dynamicsWorld->updateSingleAabb(b);
                    b->activate();
                    break;
                }
                case ObjectCommandType::COMMAND_TYPE_SET_LINEAR_FACTOR:
                    b->setLinearFactor(entry.val);
                    break;
                case ObjectCommandType::COMMAND_TYPE_SET_LINEAR_VELOCITY:
                    b->setLinearVelocity(entry.val);
                    b->activate();
                    break;
                case ObjectCommandType::COMMAND_TYPE_DESTROY_BODY: {
                    if(b->isInWorld()){
                        dynamicsWorld->removeRigidBody(b);
                    }
                    std::unique_lock<std::mutex> outputDestructionLock(outputDestructionBufferMutex);
                    outputDestructionBuffer.push_back({b, ObjectDestructionType::DESTRUCTION_TYPE_BODY});
                    break;
                }
                default:{
                    break;
                }
            }
        }

        inputObjectCommandBuffer.clear();
    }

    void DynamicsWorldThreadLogic::_shiftOrigin(const btVector3& delta){
        auto* world = static_cast<btDiscreteDynamicsWorld*>(mPhysicsWorld);
        for(int i = 0; i < world->getNumCollisionObjects(); i++){
            btRigidBody* body = btRigidBody::upcast(world->getCollisionObjectArray()[i]);
            btTransform transform = body->getWorldTransform();
            transform.getOrigin() -= delta;
            body->setWorldTransform(transform);
            btTransform interpolation = body->getInterpolationWorldTransform();
            interpolation.getOrigin() -= delta;
            body->setInterpolationWorldTransform(interpolation);
            if(body->getMotionState()){
                auto* motion = static_cast<btDefaultMotionState*>(body->getMotionState());
                motion->m_startWorldTrans.getOrigin() -= delta;
                motion->setWorldTransform(transform);
            }
            world->updateSingleAabb(body);
        }
        //Local contact points and solver impulses stay valid; only their cached world positions move.
        for(int i = 0; i < mDispatcher->getNumManifolds(); i++){
            auto* manifold = mDispatcher->getManifoldByIndexInternal(i);
            for(int j = 0; j < manifold->getNumContacts(); j++){
                auto& point = manifold->getContactPoint(j);
                point.m_positionWorldOnA -= delta;
                point.m_positionWorldOnB -= delta;
            }
        }
        mOrigin += delta;
        std::lock_guard<std::mutex> lock(outputBufferMutex);
        for(auto& entry : outputBuffer) entry.pos -= delta;
        for(auto& entry : outputImpactBuffer) entry.pos -= delta;
        outputOrigin = mOrigin;
        statistics.shifts++;
    }

    void DynamicsWorldThreadLogic::_notifyBodyMoved(btRigidBody *body){
        mMovedBodies.push_back(body);
    }


    void DynamicsWorldThreadLogic::updateOutputBuffer(){
        std::unique_lock<std::mutex> outputLock(outputBufferMutex);

        outputBuffer.clear();
        outputOrigin = mOrigin;

        for(btRigidBody* i : mMovedBodies){
            btTransform trans;
            i->getMotionState()->getWorldTransform(trans);
            //Here I duplicate the transform data, because otherwise it's a potential race condition.
            //The motion state should only be written or read by the physics thread.
            outputBuffer.push_back({i, trans.getOrigin(), trans.getRotation(), i->getLinearVelocity()});
        }

        mMovedBodies.clear();

        _collectImpacts();
    }

    void DynamicsWorldThreadLogic::_collectImpacts(){
        const float threshold = impactThreshold.load();
        if(threshold < 0.0f) return;

        const int numManifolds = mDispatcher->getNumManifolds();
        for(int i = 0; i < numManifolds; i++){
            const btPersistentManifold* manifold = mDispatcher->getManifoldByIndexInternal(i);
            btRigidBody* a = btRigidBody::upcast(const_cast<btCollisionObject*>(manifold->getBody0()));
            btRigidBody* b = btRigidBody::upcast(const_cast<btCollisionObject*>(manifold->getBody1()));

            //A sleeping pair isn't stepped, so its manifold still holds whatever the last step left in it.
            if(!a->isActive() && !b->isActive()) continue;

            //A new point isn't a hit on its own, as a sliding body makes them constantly. What decides it is how hard the
            //solver had to work on them, which bullet writes back to the point during the step.
            btScalar impulse = 0;
            const btManifoldPoint* strongest = 0;
            for(int p = 0; p < manifold->getNumContacts(); p++){
                const btManifoldPoint& point = manifold->getContactPoint(p);
                if(point.getLifeTime() > 1) continue;

                impulse += point.getAppliedImpulse();
                if(!strongest || point.getAppliedImpulse() > strongest->getAppliedImpulse()) strongest = &point;
            }
            if(!strongest) continue;

            //The change in speed of the lighter body, so that the same number means the same hit at any mass. A contact
            //just resting reads one step's worth of gravity.
            const float strength = impulse * btMax(a->getInvMass(), b->getInvMass());
            if(strength <= threshold) continue;

            outputImpactBuffer.push_back({a, b, strongest->getPositionWorldOnB(), strength});
        }
    }

    void DynamicsWorldThreadLogic::constructWorld(){
        AV_INFO("Creating dynamics world.")

        mCollisionConfiguration = new btDefaultCollisionConfiguration();
        mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);
        mOverlappingPairCache = new btDbvtBroadphase();
        mSolver = new btSequentialImpulseConstraintSolver;
        btDiscreteDynamicsWorld* dynWorld = new btDiscreteDynamicsWorld(mDispatcher, mOverlappingPairCache, mSolver, mCollisionConfiguration);

        dynWorld->setGravity(btVector3(0, -9.8, 0));
        mPhysicsWorld = dynWorld;
    }

    void DynamicsWorldThreadLogic::destroyWorld(){
        AV_INFO("Destroying dynamics world.");

        delete mPhysicsWorld;
        delete mSolver;
        delete mOverlappingPairCache;
        delete mDispatcher;
        delete mCollisionConfiguration;

        mPhysicsWorld = 0;
        mMovedBodies.clear();
        mOrigin.setValue(0, 0, 0);
        mWorldDestroyComplete = true;
    }
}
