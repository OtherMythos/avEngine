#include "CollisionWorldThreadLogic.h"

#include "btBulletDynamicsCommon.h"

#include "Logger/Log.h"

namespace AV{
    CollisionWorldThreadLogic::CollisionWorldThreadLogic(){

    }

    CollisionWorldThreadLogic::~CollisionWorldThreadLogic(){

    }

    void CollisionWorldThreadLogic::updateWorld(){
        mPhysicsWorld->performDiscreteCollisionDetection();
    }

    void CollisionWorldThreadLogic::constructWorld(){
        AV_INFO("Creating collision world.")
        mBroadphaseCollision = new btDbvtBroadphase();
        mCollisionWorldConfiguration = new btDefaultCollisionConfiguration();
        mCollisionDispatcher = new btCollisionDispatcher(mCollisionWorldConfiguration);

        mPhysicsWorld = new btCollisionWorld(mCollisionDispatcher, mBroadphaseCollision, mCollisionWorldConfiguration);
    }

    void CollisionWorldThreadLogic::destroyWorld(){
        AV_INFO("Destroying collision world.");
        delete mPhysicsWorld;

        delete mCollisionDispatcher;
        delete mCollisionWorldConfiguration;
        delete mBroadphaseCollision;

        mPhysicsWorld = 0;
        mWorldDestroyComplete = true;
    }
}
