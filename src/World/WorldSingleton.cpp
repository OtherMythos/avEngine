#include "WorldSingleton.h"

namespace AV {
    World* WorldSingleton::_world = 0;
    SlotPosition WorldSingleton::_origin = SlotPosition();
}
