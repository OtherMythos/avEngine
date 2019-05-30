#include "PhysicsShapeManager.h"

#include "btBulletDynamicsCommon.h"

namespace AV{
    PhysicsShapeManager::PhysicsShapeManager(){
        
    }
    
    PhysicsShapeManager::~PhysicsShapeManager(){
        
    }
    
    btCollisionShape* PhysicsShapeManager::getBoxShape(btVector3 extends){
        std::vector<ShapeEntry>& vec = mShapeMap[PhysicsShapeType::CubeShape];
        btCollisionShape* shape = _findShapeInList(vec, extends);
        
        //The shape already exists 
        if(shape) return shape;
        
        //A new shape needs to be created.
        //In future wrap this around the shared pointer.
        shape = (btCollisionShape*)(new btBoxShape(extends));
        vec.push_back(ShapeEntry(extends, shape));
        
        return shape;
    }
    
    btCollisionShape* PhysicsShapeManager::_findShapeInList(const std::vector<ShapeEntry>& vec, btVector3 shapeDetails){
        for(const ShapeEntry& e : vec){
            if(e.first == shapeDetails)
                return e.second;
        }
        
        return 0;
    }
}
