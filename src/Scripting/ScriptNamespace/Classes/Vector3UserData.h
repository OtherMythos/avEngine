#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class Vector3UserData{
    public:
        Vector3UserData() = delete;
        ~Vector3UserData() = delete;

        static void setupTable(HSQUIRRELVM vm);

        static void vector3ToUserData(HSQUIRRELVM vm, const Ogre::Vector3& vec);
        static UserDataGetResult readVector3FromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Vector3* outVec);

    private:
        enum class OperationType{
            Add, Subtract, Multiply, Divide
        };
        static SQInteger setMetamethod(HSQUIRRELVM vm);
        static SQInteger getMetamethod(HSQUIRRELVM vm);
        static SQInteger vector3ToString(HSQUIRRELVM vm);
        static SQInteger unaryMinusMetamethod(HSQUIRRELVM vm);
        static SQInteger addMetamethod(HSQUIRRELVM vm);
        static SQInteger minusMetamethod(HSQUIRRELVM vm);
        static SQInteger multiplyMetamethod(HSQUIRRELVM vm);
        static SQInteger divideMetamethod(HSQUIRRELVM vm);
        static SQInteger vector3Compare(HSQUIRRELVM vm);
        static SQInteger moveTowards(HSQUIRRELVM vm);
        static SQInteger perpendicular(HSQUIRRELVM vm);

        static SQInteger xy(HSQUIRRELVM vm);
        static SQInteger xz(HSQUIRRELVM vm);
        static SQInteger copy(HSQUIRRELVM vm);

        static SQInteger normalise(HSQUIRRELVM vm);
        static SQInteger normalisedCopy(HSQUIRRELVM vm);
        static SQInteger distance(HSQUIRRELVM vm);
        static SQInteger crossProduct(HSQUIRRELVM vm);
        static SQInteger dotProduct(HSQUIRRELVM vm);
        static SQInteger absDotProduct(HSQUIRRELVM vm);

        static SQInteger makeCeil(HSQUIRRELVM vm);
        static SQInteger makeFloor(HSQUIRRELVM vm);

        static SQInteger _operatorMetamethod(HSQUIRRELVM vm, OperationType opType);

        static SQObject vector3DelegateTableObject;

        static SQInteger createVector3(HSQUIRRELVM vm);

        //Read a user data and return
        static UserDataGetResult _readVector3PtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Vector3** outVec);
    };
}
