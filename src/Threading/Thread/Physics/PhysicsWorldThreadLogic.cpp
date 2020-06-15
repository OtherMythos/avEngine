#include "PhysicsWorldThreadLogic.h"

namespace AV{
    PhysicsWorldThreadLogic::PhysicsWorldThreadLogic(){

    }

    PhysicsWorldThreadLogic::~PhysicsWorldThreadLogic(){

    }

    void PhysicsWorldThreadLogic::checkWorldConstructDestruct(bool worldShouldExist, int currentWorldVersion){

        //To address a race condition.
        //The thread has to pick up when the main thread requests a new world, so there can be a delay.
        //There was a chance that if I created, destroyed, and then created a world quickly, the thread might not be aware that a world needs to be destroyed.
        //This is because the boolean would flip on quickly, so when next check the thread would think all is fine.
        //The version number takes care of this, so if the version numbers don't match the world can be destroyed and re-created fine.
        if(currentWorldVersion > mCurrentWorldVersion && worldShouldExist){
            if(mPhysicsWorld){
                destroyWorld();
            }
            constructWorld();

            mCurrentWorldVersion = currentWorldVersion;
            return;
        }

        if(!worldShouldExist && mPhysicsWorld){
            destroyWorld();
        }
    }

    bool PhysicsWorldThreadLogic::checkWorldDestroyComplete(){
        if(mWorldDestroyComplete){
            mWorldDestroyComplete = false;
            return true;
        }
        return false;
    }
}
