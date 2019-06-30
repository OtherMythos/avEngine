#pragma once

#include <memory>

namespace AV{
    class DynamicsWorld;
    class Event;

    class PhysicsManager{
    public:
        PhysicsManager();
        ~PhysicsManager();

        void initialise();

        void update();

        bool worldEventReceiver(const Event &e);

        std::shared_ptr<DynamicsWorld> getDynamicsWorld() { return mDynamicsWorld; };

    private:
        std::shared_ptr<DynamicsWorld> mDynamicsWorld;
    };
}
