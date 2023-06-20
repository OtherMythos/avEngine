#include "TextureBoxUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "OgreTextureBox.h"

#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/GPUProgramUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/GPUProgramParametersUserData.h"
#include "Scripting/ScriptNamespace/Classes/ColourValueUserData.h"

namespace AV{

    SQObject TextureBoxUserData::TextureBoxDelegateTableObject;

    void TextureBoxUserData::TextureBoxToUserData(HSQUIRRELVM vm, Ogre::TextureBox* textureBox){
        Ogre::TextureBox* pointer = (Ogre::TextureBox*)sq_newuserdata(vm, sizeof(Ogre::TextureBox));
        memcpy(pointer, textureBox, sizeof(Ogre::TextureBox));

        sq_pushobject(vm, TextureBoxDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, TextureBoxTypeTag);
        sq_setreleasehook(vm, -1, TextureBoxObjectReleaseHook);
    }

    UserDataGetResult TextureBoxUserData::readTextureBoxFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::TextureBox** outProg){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != TextureBoxTypeTag){
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outProg = (Ogre::TextureBox*)pointer;

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger TextureBoxUserData::TextureBoxObjectReleaseHook(SQUserPointer p, SQInteger size){
        Ogre::TextureBox** ptr = static_cast<Ogre::TextureBox**>(p);

        return 0;
    }

    SQInteger TextureBoxUserData::getSizeBytes(HSQUIRRELVM vm){
        Ogre::TextureBox* box;
        SCRIPT_ASSERT_RESULT(readTextureBoxFromUserData(vm, 1, &box));

        SQInteger size = box->getSizeBytes();

        sq_pushinteger(vm, size);

        return 1;
    }


    SQInteger TextureBoxUserData::getColourAt(HSQUIRRELVM vm){
        Ogre::TextureBox* box;
        SCRIPT_ASSERT_RESULT(readTextureBoxFromUserData(vm, 1, &box));

        SQInteger x, y, z;
        sq_getinteger(vm, 2, &x);
        sq_getinteger(vm, 3, &y);
        sq_getinteger(vm, 4, &z);

        SQInteger pixelFormat;
        sq_getinteger(vm, 5, &pixelFormat);
        Ogre::PixelFormatGpu format = static_cast<Ogre::PixelFormatGpu>(pixelFormat);

        Ogre::ColourValue val = box->getColourAt(x, y, z, format);
        ColourValueUserData::colourValueToUserData(vm, val);

        return 1;
    }

    SQInteger TextureBoxUserData::Write(Ogre::TextureBox* box, void* buffer, SQInteger size){
        memcpy(box->data, buffer, size);
        //_ptr += size;
        return 0;
    }
    SQInteger TextureBoxUserData::writeVal(HSQUIRRELVM v){
        Ogre::TextureBox* box;
        SCRIPT_ASSERT_RESULT(readTextureBoxFromUserData(v, 1, &box));

        SQInteger format, ti;
        SQFloat tf;
        sq_getinteger(v, 3, &format);
        switch(format) {
            case 'l': {
                SQInteger i;
                sq_getinteger(v, 2, &ti);
                i = ti;
                Write(box, &i, sizeof(SQInteger));
                      }
                break;
            case 'i': {
                SQInt32 i;
                sq_getinteger(v, 2, &ti);
                i = (SQInt32)ti;
                Write(box, &i, sizeof(SQInt32));
                      }
                break;
            case 's': {
                short s;
                sq_getinteger(v, 2, &ti);
                s = (short)ti;
                Write(box, &s, sizeof(short));
                      }
                break;
            case 'w': {
                unsigned short w;
                sq_getinteger(v, 2, &ti);
                w = (unsigned short)ti;
                Write(box, &w, sizeof(unsigned short));
                      }
                break;
            case 'c': {
                char c;
                sq_getinteger(v, 2, &ti);
                c = (char)ti;
                Write(box, &c, sizeof(char));
                          }
                break;
            case 'b': {
                unsigned char b;
                sq_getinteger(v, 2, &ti);
                b = (unsigned char)ti;
                Write(box, &b, sizeof(unsigned char));
                      }
                break;
            case 'f': {
                float f;
                sq_getfloat(v, 2, &tf);
                f = (float)tf;
                Write(box, &f, sizeof(float));
                      }
                break;
            case 'd': {
                double d;
                sq_getfloat(v, 2, &tf);
                d = tf;
                Write(box, &d, sizeof(double));
                      }
                break;
            default:
                return sq_throwerror(v, _SC("invalid format"));
            }
        return 0;
    }

    void TextureBoxUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, getSizeBytes, "getSizeBytes");
        ScriptUtils::addFunction(vm, getColourAt, "getColourAt", 5, ".iiii");
        ScriptUtils::addFunction(vm, writeVal, "writen", 3, ".ii");

        sq_resetobject(&TextureBoxDelegateTableObject);
        sq_getstackobj(vm, -1, &TextureBoxDelegateTableObject);
        sq_addref(vm, &TextureBoxDelegateTableObject);
        sq_pop(vm, 1);
    }
}
