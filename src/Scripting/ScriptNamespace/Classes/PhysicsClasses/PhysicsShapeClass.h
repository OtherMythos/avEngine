#pragma once

#include <squirrel.h>

#include "World/Physics/PhysicsTypes.h"
#include "Scripting/ScriptDataPacker.h"

namespace AV{
    /**
    A class to expose physics shapes to squirrel.
    */
    class PhysicsShapeClass{
    public:
        PhysicsShapeClass();
        ~PhysicsShapeClass();

        static void setupClass(HSQUIRRELVM vm);

        static void createInstanceFromPointer(HSQUIRRELVM vm, PhysicsTypes::ShapePtr shape);
        static PhysicsTypes::ShapePtr getPointerFromInstance(HSQUIRRELVM vm, SQInteger index);

    private:
        static SQInteger sqPhysicsShapeReleaseHook(SQUserPointer p, SQInteger size);

        static SQInteger physicsShapeCompare(HSQUIRRELVM vm);

        static ScriptDataPacker<PhysicsTypes::ShapePtr> mShapeData;

        static SQObject classObject;
    };
}
