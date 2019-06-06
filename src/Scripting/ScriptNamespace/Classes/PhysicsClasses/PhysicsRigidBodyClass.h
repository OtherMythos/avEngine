#pragma once

#include "PhysicsObjectClass.h"

namespace AV{
    class PhysicsRigidBodyClass : public PhysicsObjectClass{
    public:
        PhysicsRigidBodyClass();
        ~PhysicsRigidBodyClass();

        static void setupClass(HSQUIRRELVM vm);

    private:
        static SQObject classObject;

        static SQInteger bodyValid(HSQUIRRELVM vm);
    };
}
