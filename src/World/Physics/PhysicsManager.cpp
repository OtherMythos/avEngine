#include "PhysicsManager.h"

#include "Worlds/DynamicsWorld.h"

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
    }
}
