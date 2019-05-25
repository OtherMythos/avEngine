#include "DynamicsWorldThreadLogic.h"

#include "Logger/Log.h"
#include "btBulletDynamicsCommon.h"

namespace AV{
    DynamicsWorldThreadLogic::DynamicsWorldThreadLogic(){
        
    }
    
    void DynamicsWorldThreadLogic::updateWorld(){

        //AV_INFO("Updating dynamics world.");
        mDynamicsWorld->stepSimulation(1.0f / 60.0f, 10);
    }
    
    void DynamicsWorldThreadLogic::checkWorldConstructDestruct(bool worldShouldExist){
        if(worldShouldExist && !mDynamicsWorld){
            constructWorld();
        }
        if(!worldShouldExist && mDynamicsWorld){
            destroyWorld();
        }
    }
    
    void DynamicsWorldThreadLogic::constructWorld(){
        AV_INFO("Creating dynamics world.")
        
        mCollisionConfiguration = new btDefaultCollisionConfiguration();
    	mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);
        mOverlappingPairCache = new btDbvtBroadphase();
    	mSolver = new btSequentialImpulseConstraintSolver;
        mDynamicsWorld = new btDiscreteDynamicsWorld(mDispatcher, mOverlappingPairCache, mSolver, mCollisionConfiguration);

        mDynamicsWorld->setGravity(btVector3(0, -9.8, 0));
        
        
        btAlignedObjectArray<btCollisionShape*> collisionShapes;
        
        //Create some shapes to test it.
        //This will get deleted eventually.
        {
            btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));

            collisionShapes.push_back(groundShape);

            btTransform groundTransform;
            groundTransform.setIdentity();
            groundTransform.setOrigin(btVector3(0, -56, 0));

            btScalar mass(0.);

            //rigidbody is dynamic if and only if mass is non zero, otherwise static
            bool isDynamic = (mass != 0.f);

            btVector3 localInertia(0, 0, 0);
            if (isDynamic)
                groundShape->calculateLocalInertia(mass, localInertia);

            //using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
            btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
            btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
            btRigidBody* body = new btRigidBody(rbInfo);

            //add the body to the dynamics world
            mDynamicsWorld->addRigidBody(body);
        }

        {
            //create a dynamic rigidbody

            //btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
            btCollisionShape* colShape = new btSphereShape(btScalar(1.));
            collisionShapes.push_back(colShape);

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
            btRigidBody* body = new btRigidBody(rbInfo);

            mDynamicsWorld->addRigidBody(body);
        }
    }
    
    void DynamicsWorldThreadLogic::destroyWorld(){
        AV_INFO("Destroying dynamics world.")
        
        for (int i = mDynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
        {
            btCollisionObject* obj = mDynamicsWorld->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if (body && body->getMotionState())
            {
                delete body->getMotionState();
            }
            mDynamicsWorld->removeCollisionObject(obj);
            delete obj;
        }

        //delete collision shapes
        /*for (int j = 0; j < collisionShapes.size(); j++)
        {
            btCollisionShape* shape = collisionShapes[j];
            collisionShapes[j] = 0;
            delete shape;
        }*/
        
        delete mDynamicsWorld;
        delete mSolver;
        delete mOverlappingPairCache;
        delete mDispatcher;
        delete mCollisionConfiguration;
        
        mDynamicsWorld = 0;
    }
}
