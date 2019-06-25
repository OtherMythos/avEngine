#pragma once

#include "LinearMath/btMotionState.h"

#include "Threading/Thread/Physics/DynamicsWorldThreadLogic.h"

class btRigidBody;

namespace AV{

    /**
    A simple motion state to notify the physics thread which rigid bodies have moved in the dynamics world.
    */
    class DynamicsWorldMotionState : public btDefaultMotionState{
        friend DynamicsWorldThreadLogic;
    public:
        DynamicsWorldMotionState(const btTransform& transform) : btDefaultMotionState(transform) { }

        //Used to uniquely identify the body it is attached to.
        btRigidBody* body;

        ~DynamicsWorldMotionState(){

        }

        void setWorldTransform(const btTransform& worldTrans){
            btDefaultMotionState::setWorldTransform(worldTrans);

            //This should only ever be called by the physics thread.
            dynLogic->_notifyBodyMoved(body);
        }

        void getWorldTransform(btTransform& worldTrans) const {
            btDefaultMotionState::getWorldTransform(worldTrans);
        }

    private:
        static DynamicsWorldThreadLogic* dynLogic;
    };

}
