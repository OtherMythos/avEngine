#pragma once

#include "LinearMath/btMotionState.h"

#include "Logger/Log.h"

namespace AV{

    class DynamicsWorldMotionState : public btDefaultMotionState{
    public:
        DynamicsWorldMotionState(const btTransform& transform) : btDefaultMotionState(transform) { }

        ~DynamicsWorldMotionState(){

        }

        void setWorldTransform(const btTransform& worldTrans){
            btDefaultMotionState::setWorldTransform(worldTrans);
        }

        void getWorldTransform(btTransform& worldTrans) const {
            btDefaultMotionState::getWorldTransform(worldTrans);
        }
    };

}
