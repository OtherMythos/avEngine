#pragma once

#include <vector>
#include <map>
#include <LinearMath/btVector3.h>

class btCollisionShape;

namespace AV{
    class PhysicsShapeManager{
    public:
        PhysicsShapeManager();
        ~PhysicsShapeManager();
        
        btCollisionShape* getBoxShape(btVector3 extends);
        
    private:
        typedef std::pair<btVector3, btCollisionShape*> ShapeEntry;
        
        //Bullet has its own shape type enum, i.e BOX_SHAPE_PROXYTYPE.
        //I didn't decide to use them because it would involve including a header.
        enum class PhysicsShapeType{
            CubeShape
        };
        
        std::map<PhysicsShapeType, std::vector<ShapeEntry>> mShapeMap;
        
        btCollisionShape* _findShapeInList(const std::vector<ShapeEntry>& vec, btVector3 shapeDetails);
    };
}
