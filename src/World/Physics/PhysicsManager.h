#pragma once

#include <memory>

namespace AV{
    class DynamicsWorld;
    
    class PhysicsManager{
    public:
        PhysicsManager();
        ~PhysicsManager();
        
        void initialise();
        
        void update();
        
        std::shared_ptr<DynamicsWorld> getDynamicsWorld() { return mDynamicsWorld; };
        
    private:
        std::shared_ptr<DynamicsWorld> mDynamicsWorld;
    };
}
