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
        assert(extends.x() >= 0);
        
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
        shape->setUserIndex(shapesVector.size());
        //I'm using the int to store the index of the shape in the vector, so I do some stuff with the pointer to amke it represent the type of shape.
        //This is used so that I later know where to delete in the vector.
        void* shapePtr = reinterpret_cast<void*>((int)shapeType);
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
        
        void* ptr = shape->getUserPointer();
        PhysicsShapeType shapeType = (PhysicsShapeType)(reinterpret_cast<size_t>(ptr));
        int index = shape->getUserIndex();
        
        //Make sure that we're not deleting something that's already a hole.
        assert(staticPtr->mShapeMap[shapeType][index].first.x() != -1);

        //All I do to remove an index is specify a negative value in the vector.
        //A negative value in the x represents a hole.
        //A positive value in the y represents the next hole index in the array. If there is no hole proceding it will be a negative number.
        btVector3 removalVector(-1, 0, 0);

        
        int &firstHole = staticPtr->mFirstArrayHole;
        //Update the array hole tracker.
        //The tracker is used to reduce search time to find a hole in the vector.
        //It essentually turns it into a free-list where the vector points to the next free index.
        if(index < firstHole){
            removalVector.setY(firstHole);
            firstHole = index;
        }else if(firstHole < 0){
            firstHole = index;
            removalVector.setY(-1);
        }else if(index > firstHole){
            //The index of the shape to be deleted is greater than the first hole.
            //We need to do a search to find the hole to replace.
            int previousSearchIndex = -1;
            int currentSearchIndex = firstHole;
            while(true){
                //Find dat hole.
                int foundIndex = staticPtr->mShapeMap[shapeType][currentSearchIndex].first.y();
                
                assert(foundIndex != index);
                if(foundIndex == -1){
                    //There is no hole preceding this one.
                    staticPtr->mShapeMap[shapeType][currentSearchIndex].first.setY(index);
                    //There is nothing for our newly created hole to point to, so make that apparent.
                    removalVector.setY(-1);
                    
                    break;
                }else if(foundIndex < index){
                    //Hasn't been found yet, keep searching.
                    previousSearchIndex = currentSearchIndex;
                    currentSearchIndex = foundIndex;
                }
                else if(foundIndex > index){
                    //The correct point has been found, replace that.
                    
                    //Make the previous point to our newly deleted index.
                    staticPtr->mShapeMap[shapeType][currentSearchIndex].first.setY(index);
                    //Make our newly deleted hole point to the next available hole.
                    removalVector.setY(foundIndex);
                    
                    break;
                }
            }
        }
        
        staticPtr->mShapeMap[shapeType][index].first = removalVector;
    }
}
