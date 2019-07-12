#include "PhysicsShapeManager.h"

#include "btBulletDynamicsCommon.h"
#include "PhysicsBodyDestructor.h"

#include "Logger/Log.h"

namespace AV{

    PhysicsShapeManager::ShapeMapType PhysicsShapeManager::mShapeMap;


    void PhysicsShapeManager::shutdown(){
        //TODO tidy this up with a proper loop.
        for(ShapeEntry& s : mShapeMap[PhysicsShapeType::CubeShape].second){
            if(s.second.expired()) continue;
            s.second.reset();
        }

        for(ShapeEntry& s : mShapeMap[PhysicsShapeType::SphereShape].second){
            if(s.second.expired()) continue;
            s.second.reset();
        }

        mShapeMap.clear();
    }

    btCollisionShape* PhysicsShapeManager::_createShape(PhysicsShapeType shapeType, btVector3 extends){
        switch(shapeType){
            case PhysicsShapeType::CubeShape:
                return new btBoxShape(extends);
                break;
            case PhysicsShapeType::SphereShape:
                return new btSphereShape(extends.x());
                break;
            case PhysicsShapeType::CapsuleShape:
                return new btCapsuleShape(extends.x(), extends.y());
                break;
            default:
                //Box shape is the default because I'm secretly a pro boxer (I'm not)
                return new btBoxShape(extends);
                break;
        }
    }

    PhysicsShapeManager::ShapePtr PhysicsShapeManager::_getShape(PhysicsShapeType shapeType, btVector3 extends){
        assert(extends.x() >= 0);

        auto& shapesVectorPair = mShapeMap[shapeType];

        auto& shapesVector = shapesVectorPair.second;

        //If there are no entries in the vector then there should be no first hole.
        //This was put there more for setup than anything, as I need to make sure the list is initialised with -1.
        if(shapesVector.size() <= 0){
            shapesVectorPair.first = -1;
        }

        int shapesVectorFirstHole = shapesVectorPair.first;

        ShapePtr sharedPtr;

        for(const ShapeEntry& e : shapesVector){
            //In this case it's a hole.
            if(e.first.x() < 0) continue;
            if(e.first == extends){
                //Shouldn't need to check if lockable, as it would only not be lockable if a hole.
                sharedPtr = e.second.lock();
                return sharedPtr;
            }
        }

        //If not found in the initial search, create the new object.
        btCollisionShape *shape = _createShape(shapeType, extends);
        int targetHole = _determineListPosition(shapesVector, shapesVectorPair.first);

        int indexVal = (targetHole != -1) ? targetHole : shapesVector.size();

        shape->setUserIndex(indexVal);
        //I'm using the int to store the index of the shape in the vector, so I do some stuff with the pointer to make it represent the type of shape.
        //This is used so that I later know where to delete in the vector.
        void* shapePtr = reinterpret_cast<void*>((int)shapeType);
        shape->setUserPointer(shapePtr);

        sharedPtr = ShapePtr(shape, [](btCollisionShape *shape) {
            PhysicsShapeManager::_removeShape(shape);
            PhysicsBodyDestructor::destroyCollisionShape(shape);
    		//delete shape;
        });
        //Create a weak pointer and insert it into the vector.
        WeakShapePtr weak(sharedPtr);

        if(targetHole == -1) shapesVector.push_back({extends, weak});
        else{
            shapesVector[targetHole] = ShapeEntry(extends, weak);
        }

        return sharedPtr;
    }

    bool PhysicsShapeManager::shapeExists(PhysicsShapeType shapeType, btVector3 shape){
        auto& shapesVectorPair = mShapeMap[shapeType];

        int shapesVectorFirstHole = shapesVectorPair.first;
        auto& shapesVector = shapesVectorPair.second;

        if(shapesVector.size() <= 0) return false;

        for(const ShapeEntry& e : shapesVector){
            if(e.first == shape && !e.second.expired()) return true;
        }

        return false;
    }

    PhysicsShapeManager::ShapePtr PhysicsShapeManager::getSphereShape(btScalar radius){
        return _getShape(PhysicsShapeType::SphereShape, btVector3(radius, 0, 0));
    }

    PhysicsShapeManager::ShapePtr PhysicsShapeManager::getBoxShape(btVector3 extends){
        return _getShape(PhysicsShapeType::CubeShape, extends);
    }

    PhysicsShapeManager::ShapePtr PhysicsShapeManager::getCapsuleShape(btScalar radius, btScalar height){
        return _getShape(PhysicsShapeType::CapsuleShape, btVector3(radius, height, 0));
    }

    int PhysicsShapeManager::_determineListPosition(std::vector<ShapeEntry>& vec, int& vecFirstHole){
        if(vecFirstHole >= 0){
            int currentHole = vecFirstHole;

            vecFirstHole = vec[currentHole].first.y();

            return currentHole;
        }

        return -1;
    }

    PhysicsShapeManager::PhysicsShapeType PhysicsShapeManager::_determineShapeType(void* ptr){
        //Do a bitwise operation to remove the final bit of the 32 bit number. The final bit is used to represent whether anything was ever attached to this shape.
        int obtained = ((uintptr_t)ptr & 0x7FFFFFFF);

        return (PhysicsShapeType)obtained;
    }

    void PhysicsShapeManager::_removeShape(btCollisionShape* shape){
        PhysicsShapeType shapeType = _determineShapeType(shape->getUserPointer());

        int index = shape->getUserIndex();

        auto& shapeVector = mShapeMap[shapeType].second;

        //Make sure that we're not deleting something that's already a hole.
        assert(shapeVector[index].first.x() != -1);

        //All I do to remove an index is specify a negative value in the vector.
        //A negative value in the x represents a hole.
        //A positive value in the y represents the next hole index in the array. If there is no hole proceding it will be a negative number.
        btVector3 removalVector(-1, 0, 0);


        int &firstHole = mShapeMap[shapeType].first;
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
                int foundIndex = shapeVector[currentSearchIndex].first.y();

                assert(foundIndex != index);
                if(foundIndex == -1){
                    //There is no hole preceding this one.
                    shapeVector[currentSearchIndex].first.setY(index);
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
                    shapeVector[currentSearchIndex].first.setY(index);
                    //Make our newly deleted hole point to the next available hole.
                    removalVector.setY(foundIndex);

                    break;
                }
            }
        }

        shapeVector[index].first = removalVector;
        shapeVector[index].second.reset();
    }
}
