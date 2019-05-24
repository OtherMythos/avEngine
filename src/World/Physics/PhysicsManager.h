#pragma once

#include <memory>

namespace AV{
    class DynamicsWorld;
    
    class PhysicsManager{
    public:
        PhysicsManager();
        ~PhysicsManager();
        
        void initialise();
        
    private:
        std::shared_ptr<DynamicsWorld> mDynamicsWorld;
    };
}
