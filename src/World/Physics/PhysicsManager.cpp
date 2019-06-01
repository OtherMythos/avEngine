#include "PhysicsManager.h"

#include "Worlds/DynamicsWorld.h"

#include "PhysicsShapeManager.h"

namespace AV{
    PhysicsManager::PhysicsManager(){
        initialise();
    }
    
    PhysicsManager::~PhysicsManager(){
        
    }
    
    void PhysicsManager::update(){
        mDynamicsWorld->update();
    }
    
    void PhysicsManager::initialise(){
        mDynamicsWorld = std::make_shared<DynamicsWorld>();
        
        mShapeManager = std::make_shared<PhysicsShapeManager>();
        
        //Testing code
        PhysicsShapeManager::ShapePtr first = 
        mShapeManager->getBoxShape(btVector3(10, 20, 30));
        
        assert(mShapeManager->getBoxShape(btVector3(10, 20, 30)) == first);
        
        assert(mShapeManager->getBoxShape(btVector3(10, 20, 40)) != first);
    }
}
