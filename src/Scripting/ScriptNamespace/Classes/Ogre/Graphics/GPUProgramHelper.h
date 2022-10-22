#pragma once

#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptNamespace/Classes/Vector2UserData.h"
#include "Scripting/ScriptNamespace/Classes/ColourValueUserData.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "Ogre.h"

namespace AV{
    /**
    Shared helper functions for script GPU exposure.
    */
    class GPUProgramHelper{
    public:
        static SQInteger getNamedConstant(HSQUIRRELVM vm){

        }

        static SQInteger setNamedConstant(HSQUIRRELVM vm, Ogre::GpuProgramParametersSharedPtr params){
            const SQChar *name;
            sq_getstring(vm, 2, &name);

            //TODO check the provided constant matches

            SQObjectType type = sq_gettype(vm, 3);
            if(type == OT_USERDATA){
                SQUserPointer outPtr = 0;
                sq_gettypetag(vm, 3, &outPtr);
                if(outPtr == Vector3TypeTag){
                    Ogre::Vector3 vec;
                    SCRIPT_ASSERT_RESULT(Vector3UserData::readVector3FromUserData(vm, 3, &vec));
                    params->setNamedConstant(name, vec);
                }
                else if(outPtr == Vector2TypeTag){
                    Ogre::Vector2 vec;
                    SCRIPT_ASSERT_RESULT(Vector2UserData::readVector2FromUserData(vm, 3, &vec));
                    params->setNamedConstant(name, vec);
                }
                else if(outPtr == ColourValueTypeTag){
                    Ogre::ColourValue col;
                    SCRIPT_ASSERT_RESULT(ColourValueUserData::readColourValueFromUserData(vm, 3, &col));
                    params->setNamedConstant(name, col);
                }
            }
            else if(type == OT_INTEGER){
                SQInteger intVal;
                sq_getinteger(vm, 3, &intVal);
                params->setNamedConstant(name, (int)intVal);
            }
            else if(type == OT_FLOAT){
                SQFloat floatVal;
                sq_getfloat(vm, 3, &floatVal);
                params->setNamedConstant(name, (Ogre::Real)floatVal);
            }
            else{
                return sq_throwerror(vm, "Unknown type passed as value.");
            }

            return 0;
        }

    };
}
