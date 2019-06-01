#include "PhysicsShapeClass.h"

namespace AV{
    PhysicsShapeClass::PhysicsShapeClass(){
        
    }
    
    PhysicsShapeClass::~PhysicsShapeClass(){
        
    }
    
    void PhysicsShapeClass::setupClass(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("PhysicsShape"), -1);
        sq_newclass(vm, 0);
        
        sq_newslot(vm, -3, false);        
    }
}
