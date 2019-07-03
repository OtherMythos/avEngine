#pragma once

#include "World/Physics/PhysicsBodyConstructor.h"

namespace AV{
    class PhysicsShapeManager;
}

class PhysicsBodyConstructorMock : public AV::PhysicsBodyConstructor{
public:
    PhysicsBodyConstructorMock(std::shared_ptr<AV::PhysicsShapeManager> shapeMgr) : PhysicsBodyConstructor(shapeMgr) {};
};
