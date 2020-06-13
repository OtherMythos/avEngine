#include "ScriptGetterUtils.h"

#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"

namespace AV{
    bool ScriptGetterUtils::vector3Read(HSQUIRRELVM vm, Ogre::Vector3* outVec){
        SQInteger size = sq_gettop(vm);

        if(size == 2){
            //Vector3
            if(Vector3UserData::readVector3FromUserData(vm, -1, outVec)) return true;

            SlotPosition pos;
            bool success = SlotPositionClass::getSlotFromInstance(vm, -1, &pos);
            if(success){
                *outVec = pos.toOgre();
                return true;
            }
            return false;
        }else if(size == 4){
            //Regular

            bool success = true;
            SQFloat x, y, z;
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -1, &z));
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -2, &y));
            success &= SQ_SUCCEEDED(sq_getfloat(vm, -3, &x));
            if(!success) return false;

            *outVec = Ogre::Vector3(x, y, z);

            sq_pop(vm, 3);
        }else return false;

        return true;
    }
}
