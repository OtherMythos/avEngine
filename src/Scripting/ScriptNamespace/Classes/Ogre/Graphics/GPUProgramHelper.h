#pragma once

#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptNamespace/Classes/Vector2UserData.h"
#include "Scripting/ScriptNamespace/Classes/ColourValueUserData.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include <sqstdblob.h>

#include "Ogre.h"

namespace AV{
    /**
    Shared helper functions for script GPU exposure.
    */
    class GPUProgramHelper{
    private:

        static const char* _getStringForConstType(Ogre::GpuConstantType constType){
            switch(constType){
                case Ogre::GpuConstantType::GCT_FLOAT1: return "float";
                case Ogre::GpuConstantType::GCT_FLOAT2: return "float2";
                case Ogre::GpuConstantType::GCT_FLOAT3: return "float3";
                case Ogre::GpuConstantType::GCT_FLOAT4: return "float4";
                case Ogre::GpuConstantType::GCT_INT1: return "int";
                case Ogre::GpuConstantType::GCT_INT2: return "int2";
                case Ogre::GpuConstantType::GCT_INT3: return "int3";
                case Ogre::GpuConstantType::GCT_INT4: return "int4";
                case Ogre::GpuConstantType::GCT_UINT1: return "uint";
                case Ogre::GpuConstantType::GCT_UINT2: return "uint2";
                case Ogre::GpuConstantType::GCT_UINT3: return "uint3";
                case Ogre::GpuConstantType::GCT_UINT4: return "uint4";
                case Ogre::GpuConstantType::GCT_BOOL1: return "bool";
                case Ogre::GpuConstantType::GCT_BOOL2: return "bool2";
                case Ogre::GpuConstantType::GCT_BOOL3: return "bool3";
                case Ogre::GpuConstantType::GCT_BOOL4: return "bool4";
                case Ogre::GpuConstantType::GCT_MATRIX_2X2: return "matrix2x2";
                case Ogre::GpuConstantType::GCT_MATRIX_2X3: return "matrix2x3";
                case Ogre::GpuConstantType::GCT_MATRIX_2X4: return "matrix2x4";
                case Ogre::GpuConstantType::GCT_MATRIX_3X2: return "matrix3x2";
                case Ogre::GpuConstantType::GCT_MATRIX_3X3: return "matrix3x3";
                case Ogre::GpuConstantType::GCT_MATRIX_3X4: return "matrix3x4";
                case Ogre::GpuConstantType::GCT_MATRIX_4X2: return "matrix4x2";
                case Ogre::GpuConstantType::GCT_MATRIX_4X3: return "matrix4x3";
                case Ogre::GpuConstantType::GCT_MATRIX_4X4: return "matrix4x4";
                default: return "unsupported";
            }
        }

        static SQInteger _throwTypeMismatchError(HSQUIRRELVM vm, Ogre::GpuConstantType shaderType, Ogre::GpuConstantType providedType){
            Ogre::String outString = "Mismatch of constant types. Shader specified type '";
            outString += _getStringForConstType(shaderType);
            outString += "' provided type was '";
            outString += _getStringForConstType(providedType);
            outString += "'.";
            return sq_throwerror(vm, outString.c_str());
        }

    public:
        static SQInteger getNamedConstant(HSQUIRRELVM vm){

        }

        static SQInteger setNamedConstant(HSQUIRRELVM vm, Ogre::GpuProgramParametersSharedPtr params){
            const SQChar *name;
            sq_getstring(vm, 2, &name);

            const Ogre::GpuConstantDefinition* constDef = params->_findNamedConstantDefinition(name);
            if(!constDef){
                Ogre::String error("Could not find const def with name ");
                error += name;
                return sq_throwerror(vm, error.c_str());
            }

            SQObjectType type = sq_gettype(vm, 3);
            if(type == OT_USERDATA){
                SQUserPointer outPtr = 0;
                sq_gettypetag(vm, 3, &outPtr);
                if(outPtr == Vector3TypeTag){
                    if(constDef->constType == Ogre::GpuConstantType::GCT_FLOAT3){
                        Ogre::Vector3 vec;
                        SCRIPT_ASSERT_RESULT(Vector3UserData::readVector3FromUserData(vm, 3, &vec));
                        params->_writeRawConstant(constDef->physicalIndex, vec);
                    }else{
                        return _throwTypeMismatchError(vm, constDef->constType, Ogre::GpuConstantType::GCT_FLOAT3);
                    }
                }
                else if(outPtr == Vector2TypeTag){
                    if(constDef->constType == Ogre::GpuConstantType::GCT_FLOAT2){
                        Ogre::Vector2 vec;
                        SCRIPT_ASSERT_RESULT(Vector2UserData::readVector2FromUserData(vm, 3, &vec));
                        params->_writeRawConstant(constDef->physicalIndex, vec);
                    }else{
                        return _throwTypeMismatchError(vm, constDef->constType, Ogre::GpuConstantType::GCT_FLOAT2);
                    }
                }
                else if(outPtr == ColourValueTypeTag){
                    if(constDef->constType == Ogre::GpuConstantType::GCT_FLOAT4){
                        Ogre::ColourValue col;
                        SCRIPT_ASSERT_RESULT(ColourValueUserData::readColourValueFromUserData(vm, 3, &col));
                        params->_writeRawConstant(constDef->physicalIndex, col, 1);
                    }else{
                        return _throwTypeMismatchError(vm, constDef->constType, Ogre::GpuConstantType::GCT_FLOAT4);
                    }
                }
            }
            else if(type == OT_INTEGER){
                SQInteger intVal;
                sq_getinteger(vm, 3, &intVal);
                if(constDef->isInt()){
                    params->_writeRawConstant(constDef->physicalIndex, (int)intVal);
                }
                else if(constDef->isUnsignedInt()){
                    params->_writeRawConstant(constDef->physicalIndex, (unsigned int)intVal);
                }
                else{
                    return _throwTypeMismatchError(vm, constDef->constType, Ogre::GpuConstantType::GCT_INT1);
                }
            }
            else if(type == OT_FLOAT){
                SQFloat floatVal;
                sq_getfloat(vm, 3, &floatVal);
                if(constDef->isFloat()){
                    params->_writeRawConstant(constDef->physicalIndex, (Ogre::Real)floatVal);
                }
                else{
                    return _throwTypeMismatchError(vm, constDef->constType, Ogre::GpuConstantType::GCT_FLOAT1);
                }
            }
            else if(type == OT_BOOL){
                SQBool boolValue;
                sq_getbool(vm, 3, &boolValue);
                if(constDef->isBool()){
                    params->_writeRawConstant(constDef->physicalIndex, (int)boolValue);
                }
                else{
                    return _throwTypeMismatchError(vm, constDef->constType, Ogre::GpuConstantType::GCT_BOOL1);
                }
            }
            else if(type == OT_INSTANCE){
                SQUserPointer blobData = 0;
                if(SQ_FAILED(sqstd_getblob(vm, 3, &blobData))){
                    return sq_throwerror(vm, "Unknown type passed as value.");
                }
                SQInteger blobSize = sqstd_getblobsize(vm, 3);

                size_t elemSize = Ogre::GpuConstantDefinition::getElementSize(constDef->constType, false);
                elemSize *= constDef->arraySize;
                if(blobSize / 4 > elemSize){
                    return sq_throwerror(vm, "Blob is larger than constant size.");
                }

                if(constDef->isInt()){
                    params->_writeRawConstants(constDef->physicalIndex, (const int*)blobData, elemSize);
                }
                else if(constDef->isUnsignedInt()){
                    params->_writeRawConstants(constDef->physicalIndex, (Ogre::uint*)blobData, elemSize);
                }
                else if(constDef->isFloat()){
                    params->_writeRawConstants(constDef->physicalIndex, (float*)blobData, elemSize);
                }else{
                    return sq_throwerror(vm, "Invalid constant type for setting blob value.");
                }
            }
            else{
                return sq_throwerror(vm, "Unknown type passed as value.");
            }

            return 0;
        }

    };
}
