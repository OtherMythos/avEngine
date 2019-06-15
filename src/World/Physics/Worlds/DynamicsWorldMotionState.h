#pragma once

#include "LinearMath/btMotionState.h"

#include "Logger/Log.h"

#include "Threading/Thread/Physics/DynamicsWorldThreadLogic.h"

class btRigidBody;

namespace AV{

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
