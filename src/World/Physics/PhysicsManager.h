#pragma once

#include <memory>

namespace AV{
    class DynamicsWorld;
    class PhysicsShapeManager;
    
    class PhysicsManager{
    public:
        PhysicsManager();
        ~PhysicsManager();
        
        void initialise();
        
        void update();
        
        std::shared_ptr<DynamicsWorld> getDynamicsWorld() { return mDynamicsWorld; };
        
    private:
        std::shared_ptr<DynamicsWorld> mDynamicsWorld;
        
        std::shared_ptr<PhysicsShapeManager> mShapeManager;
    };
}
