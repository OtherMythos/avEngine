#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    class ColourValueUserData{
    public:
        ColourValueUserData() = delete;
        ~ColourValueUserData() = delete;

        static void setupTable(HSQUIRRELVM vm);

        static void colourValueToUserData(HSQUIRRELVM vm, const Ogre::ColourValue& vec);
        static UserDataGetResult readColourValueFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::ColourValue* outVec);

    private:
        enum class OperationType{
            Add, Subtract, Multiply, Divide
        };
        static SQInteger setMetamethod(HSQUIRRELVM vm);
        static SQInteger getMetamethod(HSQUIRRELVM vm);
        static SQInteger colourValueToString(HSQUIRRELVM vm);
        static SQInteger addMetamethod(HSQUIRRELVM vm);
        static SQInteger minusMetamethod(HSQUIRRELVM vm);
        static SQInteger multiplyMetamethod(HSQUIRRELVM vm);
        static SQInteger divideMetamethod(HSQUIRRELVM vm);
        static SQInteger colourValueCompare(HSQUIRRELVM vm);

        static SQInteger _operatorMetamethod(HSQUIRRELVM vm, OperationType opType);

        static SQObject colourValueDelegateTableObject;

        static SQInteger createColourValue(HSQUIRRELVM vm);

        static UserDataGetResult _readColourValuePtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::ColourValue** outVec);
    };
}
