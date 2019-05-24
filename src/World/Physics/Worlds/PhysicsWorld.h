#pragma once

namespace AV{
    
    /**
    Base class for physics worlds.
    Contains logic for thread safe communication and interaction with the world.
    */
    class PhysicsWorld{
    public:
        PhysicsWorld();
        ~PhysicsWorld();
    };
}
