#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class QuaternionUserData{
    public:
        QuaternionUserData() = delete;
        ~QuaternionUserData() = delete;

        static void setupTable(HSQUIRRELVM vm);

        static void quaternionToUserData(HSQUIRRELVM vm, const Ogre::Quaternion& quat);
        static UserDataGetResult readQuaternionFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Quaternion* outQuat);

    private:
        enum class OperationType{
            Add, Subtract
        };
        static SQInteger setMetamethod(HSQUIRRELVM vm);
        static SQInteger getMetamethod(HSQUIRRELVM vm);
        static SQInteger quaternionToString(HSQUIRRELVM vm);
        static SQInteger unaryMinusMetamethod(HSQUIRRELVM vm);
        static SQInteger addMetamethod(HSQUIRRELVM vm);
        static SQInteger minusMetamethod(HSQUIRRELVM vm);
        static SQInteger multiplyMetamethod(HSQUIRRELVM vm);
        static SQInteger copy(HSQUIRRELVM vm);

        static SQInteger slerp(HSQUIRRELVM vm);
        static SQInteger nlerp(HSQUIRRELVM vm);

        static SQInteger _operatorMetamethod(HSQUIRRELVM vm, OperationType opType);
        static SQInteger _operatorMultiplyMetamethod(HSQUIRRELVM vm);
        static SQInteger _slerpNlerp(HSQUIRRELVM vm, bool slerp);

        static SQObject quaternionDelegateTableObject;

        static SQInteger createQuaternion(HSQUIRRELVM vm);

        //Read a user data and return
        static UserDataGetResult _readQuaternionPtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Quaternion** outQuat);
    };
}
