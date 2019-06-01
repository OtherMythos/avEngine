#pragma once

#include <squirrel.h>

namespace AV{
    /**
    A class to expose physics shapes to squirrel.
    */
    class PhysicsShapeClass{
    public:
        PhysicsShapeClass();
        ~PhysicsShapeClass();
        
        static void setupClass(HSQUIRRELVM vm);
        
    private:
    };
}
