#pragma once

#include <map>
#include <vector>
#include <memory>
#include <LinearMath/btVector3.h>

#include "PhysicsTypes.h"

class btCollisionShape;
class btHeightfieldTerrainShape;

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
    public:
        //Bullet has its own shape type enum, i.e BOX_SHAPE_PROXYTYPE.
        //I didn't decide to use them because it would involve including a header.
        enum class PhysicsShapeType{
            CubeShape,
            SphereShape,
            CapsuleShape
        };

    public:
        PhysicsShapeManager() = delete;
        ~PhysicsShapeManager() = delete;

        static void shutdown();

        static PhysicsTypes::ShapePtr getBoxShape(btVector3 extends);
        static PhysicsTypes::ShapePtr getSphereShape(btScalar radius);
        static PhysicsTypes::ShapePtr getCapsuleShape(btScalar radius, btScalar height);
        static btHeightfieldTerrainShape* getTerrainShape(int heightWidth, int heightLength, const void* data, btScalar minHeight, btScalar maxHeight, float localScaling);

        /**
        A function used for testing.
        */
        static bool shapeExists(PhysicsShapeType type, btVector3 shape);

        static void _removeShape(btCollisionShape* shape);

    private:
        typedef std::weak_ptr<btCollisionShape> WeakShapePtr;
        typedef std::pair<btVector3, WeakShapePtr> ShapeEntry;

        static PhysicsTypes::ShapePtr _getShape(PhysicsShapeType shapeType, btVector3 extends);
        static btCollisionShape* _createShape(PhysicsShapeType shapeType, btVector3 extends);

        typedef std::map<PhysicsShapeType, std::pair<int, std::vector<ShapeEntry>> > ShapeMapType;
        static ShapeMapType mShapeMap;

        /**
        Determine where in the free list the shape should be inserted.
        This will alter the list to account for that.

        @return
        -1 if there is no hole in the vector, otherwise an index to a hole in the vector.
        */
        static int _determineListPosition(std::vector<ShapeEntry>& vec, int& vecFirstHole);

        static PhysicsShapeType _determineShapeType(void* ptr);
    };
}
