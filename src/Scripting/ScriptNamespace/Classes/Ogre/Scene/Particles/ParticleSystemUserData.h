#pragma once

#include <squirrel.h>

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "System/Util/DataPacker.h"

namespace Ogre{
    class ParticleSystem;
}

namespace AV{
    /**
    A class to expose Particle systems to squirrel.
    Particle systems are version counted by squirrel and destroyed when those references reach 0.
    */
    class ParticleSystemUserData{
    public:
        ParticleSystemUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm, SQObject* obj);

        static void createUserDataFromPointer(HSQUIRRELVM vm, Ogre::ParticleSystem* system);
        static UserDataGetResult getPointerFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::ParticleSystem** outObject);

    private:
        static SQObject particleSystemDelegateTable;
        static SQInteger sqPhysicsShapeReleaseHook(SQUserPointer p, SQInteger size);

        static SQInteger fastForward(HSQUIRRELVM vm);
    };
}
