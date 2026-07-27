#include "ComponentLogic.h"

namespace AV{
    EntityManager* ComponentLogic::entityManager = 0;
    //I store the entityx instance here.
    //Really there were few ways to get around this.
    //The entity manager can't have a function to get an entity handle as a public function because then the encapsulation is gone.
    //I can at least share this pointer here with some 'trusted' classes to get the work done.
    //There shouldn't be many places where I need to do this.
    entityx::EntityX* ComponentLogic::entityXManager = 0;
}
