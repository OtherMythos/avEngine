#pragma once

#include <mutex>
#include <vector>

#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>

#include "World/Slot/SlotPosition.h"
#include <atomic>

class btCollisionWorld;

namespace AV{
    class PhysicsWorld;

    class PhysicsWorldThreadLogic{
    public:
        PhysicsWorldThreadLogic();
        ~PhysicsWorldThreadLogic();

        /**
        Update the world for a single thread.
        */
        virtual void updateWorld() = 0;

        void checkWorldConstructDestruct(bool worldShouldExist, int currentWorldVersion);
        /**
        Check if the destruction of the world has finished.
        This function is intended to be called by the main thread.
        If mWorldDestroyComplete is true this function will set it back to false afterwards.
        */
        bool checkWorldDestroyComplete();

    protected:
        virtual void constructWorld() = 0;
        virtual void destroyWorld() = 0;


    protected:
        //Whether the world was destroyed completely. Used by the PhysicsBodyDestructor to coordinate world shape removal.
        std::atomic<bool> mWorldDestroyComplete;

        btCollisionWorld* mPhysicsWorld = 0;
        int mCurrentWorldVersion = 0;

    public:
        btVector3 worldOriginChangeOffset;
        SlotPosition worldOriginChangeNewPosition;
        bool worldShifted = false;
    };
}
