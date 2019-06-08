#pragma once

#include <squirrel.h>

#include "World/Physics/PhysicsShapeManager.h"
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

        static void createClassFromPointer(HSQUIRRELVM vm, PhysicsShapeManager::ShapePtr shape);

    private:
        static SQInteger sqPhysicsShapeReleaseHook(SQUserPointer p, SQInteger size);

        static ScriptDataPacker<PhysicsShapeManager::ShapePtr> mShapeData;

        static SQObject classObject;
    };
}
