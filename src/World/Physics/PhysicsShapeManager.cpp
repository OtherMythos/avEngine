#include "PhysicsShapeManager.h"

#include "btBulletDynamicsCommon.h"

#include "Logger/Log.h"

namespace AV{
    PhysicsShapeManager* PhysicsShapeManager::staticPtr = 0;
    
    PhysicsShapeManager::PhysicsShapeManager(){
        PhysicsShapeManager::staticPtr = this;
    }
    
    PhysicsShapeManager::~PhysicsShapeManager(){
        PhysicsShapeManager::staticPtr = 0;
    }
    
    PhysicsShapeManager::ShapePtr PhysicsShapeManager::getBoxShape(btVector3 extends){
        PhysicsShapeType shapeType = PhysicsShapeType::CubeShape;
        auto& shapesVector = mShapeMap[shapeType];
        
        ShapePtr sharedPtr;
        
        for(const ShapeEntry& e : shapesVector){
            if(e.first == extends){
                //Maybe do a check if it's lockable.
                sharedPtr = e.second.lock();
                return sharedPtr;
            }
        }
        
        //If not found in the initial search, create the new object.
        btCollisionShape *shape = new btBoxShape(extends);
        shape->setUserIndex((int)shapeType);
        //I'm using the int value to store the type of shape, so I do some fun stuff with the pointer to make it represent the id in the vector.
        //This is used so that I later know where to delete in the vector.
        void* shapePtr = reinterpret_cast<void*>(shapesVector.size());
        shape->setUserPointer(shapePtr);
        
        sharedPtr = ShapePtr(shape, [](btCollisionShape *shape) { 
    		std::cout << "Deleted mate" << std::endl;
            PhysicsShapeManager::_destroyShape(shape);
    		delete shape;
        });
        //Create a weak pointer and insert it into the vector.
        WeakShapePtr weak(sharedPtr);
        shapesVector.push_back({extends, weak});
        
        return sharedPtr;
    }
    
    void PhysicsShapeManager::_destroyShape(btCollisionShape* shape){
        if(!staticPtr) return;
        
        PhysicsShapeType shapeType = (PhysicsShapeType)shape->getUserIndex();
        size_t index = reinterpret_cast<size_t>(shape->getUserPointer());

        //Do something here to actually remove this index.
        staticPtr->mShapeMap[shapeType][index];
    }
}
