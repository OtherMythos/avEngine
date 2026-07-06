#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace Ogre{
    class AVHlmsListenerDispatch;
}

namespace AV{
    /**
    Script handle over the per-pass shader properties of one compositor-pass
    identifier, owned by an Hlms's AVHlmsListenerDispatch. Obtained from
    _hlms.pbs.getPass(identifier). Properties are compile-time shader-variant
    flags applied in preparePassHash when the matching pass renders; register them
    at setup, not per frame (they participate in the shader cache).
    */
    class PassPropertiesUserData{
    public:
        PassPropertiesUserData() = delete;
        ~PassPropertiesUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void passPropertiesToUserData(HSQUIRRELVM vm, Ogre::AVHlmsListenerDispatch* dispatch, uint32_t passIdentifier);

    private:
        struct PassPropertiesData{
            Ogre::AVHlmsListenerDispatch* dispatch;
            uint32_t identifier;
        };

        static UserDataGetResult _read(HSQUIRRELVM vm, SQInteger stackInx, PassPropertiesData* outData);

        static SQObject PassPropertiesDelegateTableObject;

        static SQInteger setProperty(HSQUIRRELVM vm);
        static SQInteger clearProperty(HSQUIRRELVM vm);
    };
}
