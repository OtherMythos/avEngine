#pragma once

#include <map>
#include <vector>
#include <memory>
#include <LinearMath/btVector3.h>

class btCollisionShape;

namespace AV{
    class TestModePhysicsNamespace;

    /**
    A class to manage the creation and lifetime of bullet shapes.
    
    Shapes are intended to be shared between as many objects as possible.
    This class allows easy access to them, and deals with their destruction.
    Shapes are distributed through shared pointers, so they are destroyed when nothing is them anymore.
    */
    class PhysicsShapeManager{
        friend TestModePhysicsNamespace;
    private:
        //Bullet has its own shape type enum, i.e BOX_SHAPE_PROXYTYPE.
        //I didn't decide to use them because it would involve including a header.
        enum class PhysicsShapeType{
            CubeShape,
            SphereShape,
            CapsuleShape
        };

    public:
        typedef std::shared_ptr<btCollisionShape> ShapePtr;

        PhysicsShapeManager();
        ~PhysicsShapeManager();

        ShapePtr getBoxShape(btVector3 extends);
        ShapePtr getSphereShape(btScalar radius);
        ShapePtr getCapsuleShape(btScalar radius, btScalar height);

        /**
        A function used for testing.
        */
        bool shapeExists(PhysicsShapeType type, btVector3 shape);

        static void _destroyShape(btCollisionShape* shape);

    private:
        typedef std::weak_ptr<btCollisionShape> WeakShapePtr;
        typedef std::pair<btVector3, WeakShapePtr> ShapeEntry;

        PhysicsShapeManager::ShapePtr _getShape(PhysicsShapeType shapeType, btVector3 extends);
        btCollisionShape* _createShape(PhysicsShapeType shapeType, btVector3 extends);

        std::map<PhysicsShapeType, std::pair<int, std::vector<ShapeEntry>> > mShapeMap;

        /**
        Determine where in the free list the shape should be inserted.
        This will alter the list to account for that.

        @return
        -1 if there is no hole in the vector, otherwise an index to a hole in the vector.
        */
        int _determineListPosition(std::vector<ShapeEntry>& vec, int& vecFirstHole);

        static PhysicsShapeManager* staticPtr;
    };
}
