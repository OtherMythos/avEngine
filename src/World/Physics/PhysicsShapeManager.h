#pragma once

#include <map>
#include <vector>
#include <memory>
#include <LinearMath/btVector3.h>

class btCollisionShape;

namespace AV{
    class PhysicsShapeManager{
    public:
        typedef std::shared_ptr<btCollisionShape> ShapePtr;
        
        PhysicsShapeManager();
        ~PhysicsShapeManager();
        
        ShapePtr getBoxShape(btVector3 extends);
        
        static void _destroyShape(btCollisionShape* shape);
        
    private:
        typedef std::weak_ptr<btCollisionShape> WeakShapePtr;
        typedef std::pair<btVector3, WeakShapePtr> ShapeEntry;
        
        //Bullet has its own shape type enum, i.e BOX_SHAPE_PROXYTYPE.
        //I didn't decide to use them because it would involve including a header.
        enum class PhysicsShapeType{
            CubeShape
        };
        
        std::map<PhysicsShapeType, std::vector<ShapeEntry>> mShapeMap;
        
        //Represents the index of the first hole in the array.
        //-1 means there is no hole in the array.
        size_t mFirstArrayHole = -1;
        
        static PhysicsShapeManager* staticPtr;
    };
}
