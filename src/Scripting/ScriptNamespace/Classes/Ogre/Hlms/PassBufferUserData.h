#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace Ogre{
    class AVHlmsListenerDispatch;
}

namespace AV{
    /**
    Script handle over an Hlms's script pass buffer (the custom_passBuffer float
    block owned by its AVHlmsListenerDispatch). Obtained from
    _hlms.pbs.createPassBuffer(n) / getPassBuffer(). The dispatch is engine-owned,
    so the handle just stores a raw pointer and its release hook does nothing.
    */
    class PassBufferUserData{
    public:
        PassBufferUserData() = delete;
        ~PassBufferUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void passBufferToUserData(HSQUIRRELVM vm, Ogre::AVHlmsListenerDispatch* dispatch);
        static UserDataGetResult readPassBufferFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::AVHlmsListenerDispatch** outDispatch);

    private:
        static SQObject PassBufferDelegateTableObject;

        static SQInteger setFloat(HSQUIRRELVM vm);
        static SQInteger setData(HSQUIRRELVM vm);
        static SQInteger setVec3(HSQUIRRELVM vm);
        static SQInteger setVec4(HSQUIRRELVM vm);
        static SQInteger getSize(HSQUIRRELVM vm);
    };
}
