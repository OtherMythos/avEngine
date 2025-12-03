#pragma once

#include <squirrel.h>
#include "Scripting/ScriptNamespace/ScriptDefs.h"
#include "OgreVector2.h"

namespace AV{
    class Vector2UserData{
    public:
        Vector2UserData() = delete;
        ~Vector2UserData() = delete;

        static void setupTable(HSQUIRRELVM vm);

        static void vector2ToUserData(HSQUIRRELVM vm, const Ogre::Vector2& vec);
        static UserDataGetResult readVector2FromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Vector2* outVec);

    private:
        enum class OperationType{
            Add, Subtract, Multiply, Divide
        };
        static SQInteger setMetamethod(HSQUIRRELVM vm);
        static SQInteger getMetamethod(HSQUIRRELVM vm);
        static SQInteger vector2ToString(HSQUIRRELVM vm);
        static SQInteger unaryMinusMetamethod(HSQUIRRELVM vm);
        static SQInteger addMetamethod(HSQUIRRELVM vm);
        static SQInteger minusMetamethod(HSQUIRRELVM vm);
        static SQInteger multiplyMetamethod(HSQUIRRELVM vm);
        static SQInteger divideMetamethod(HSQUIRRELVM vm);
        static SQInteger vector2Compare(HSQUIRRELVM vm);
        static SQInteger copy(HSQUIRRELVM vm);
        static SQInteger perpendicular(HSQUIRRELVM vm);

        static SQInteger normalise(HSQUIRRELVM vm);
        static SQInteger normalisedCopy(HSQUIRRELVM vm);
        static SQInteger distance(HSQUIRRELVM vm);
        static SQInteger crossProduct(HSQUIRRELVM vm);
        static SQInteger length(HSQUIRRELVM vm);

        static SQInteger makeCeil(HSQUIRRELVM vm);
        static SQInteger makeFloor(HSQUIRRELVM vm);

        static SQInteger _operatorMetamethod(HSQUIRRELVM vm, OperationType opType);

        static SQObject vector2DelegateTableObject;

        static SQInteger createVector2(HSQUIRRELVM vm);

        //Read a user data and return
        static UserDataGetResult _readVector2PtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::Vector2** outVec);
    };
}
