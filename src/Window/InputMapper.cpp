#include "InputMapper.h"

#include <cassert>

namespace AV{
    InputMapper::InputMapper(){

    }

    InputMapper::~InputMapper(){

    }

    ActionHandle InputMapper::_wrapAxisTypeToHandle(ActionHandle handle, int axis){
        assert(axis >= 0 && axis <= 4); //There's four possible directions.
        ActionHandle outHandle = handle;
        outHandle |= ((unsigned char)axis << 27);
        return outHandle;
    }
}
