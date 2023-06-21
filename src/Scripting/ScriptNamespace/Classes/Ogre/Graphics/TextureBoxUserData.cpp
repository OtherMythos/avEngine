#include "TextureBoxUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "OgreTextureBox.h"

#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/GPUProgramUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/GPUProgramParametersUserData.h"
#include "Scripting/ScriptNamespace/Classes/ColourValueUserData.h"

namespace AV{

    SQObject TextureBoxUserData::TextureBoxDelegateTableObject;

    struct TextureBoxUserData::TextureBoxWrapper : public Ogre::TextureBox{
        void* seekPos;
        size_t seekCount;
        TextureBoxWrapper(const TextureBox& box) : seekCount(0) {
            memcpy(static_cast<Ogre::TextureBox*>(this), &box, sizeof(Ogre::TextureBox));
            seekPos = data;
        }
    };

    void TextureBoxUserData::TextureBoxToUserData(HSQUIRRELVM vm, Ogre::TextureBox* textureBox){
        TextureBoxWrapper* pointer = (TextureBoxWrapper*)sq_newuserdata(vm, sizeof(TextureBoxWrapper));
        new(pointer)TextureBoxWrapper(*textureBox);

        sq_pushobject(vm, TextureBoxDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, TextureBoxTypeTag);
        sq_setreleasehook(vm, -1, TextureBoxObjectReleaseHook);
    }

    UserDataGetResult TextureBoxUserData::_readTextureBoxFromUserData(HSQUIRRELVM vm, SQInteger stackInx, TextureBoxWrapper** outProg){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != TextureBoxTypeTag){
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outProg = (TextureBoxWrapper*)pointer;

        return USER_DATA_GET_SUCCESS;
    }

    UserDataGetResult TextureBoxUserData::readTextureBoxFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::TextureBox** outProg){
        TextureBoxWrapper* wrapper;
        UserDataGetResult result = _readTextureBoxFromUserData(vm, stackInx, &wrapper);
        if(result != UserDataGetResult::USER_DATA_GET_SUCCESS) return result;
        *outProg = static_cast<TextureBoxWrapper*>(wrapper);

        return result;
    }

    SQInteger TextureBoxUserData::TextureBoxObjectReleaseHook(SQUserPointer p, SQInteger size){
        Ogre::TextureBox** ptr = static_cast<Ogre::TextureBox**>(p);

        return 0;
    }

    SQInteger TextureBoxUserData::getSizeBytes(HSQUIRRELVM vm){
        TextureBoxWrapper* box;
        SCRIPT_ASSERT_RESULT(_readTextureBoxFromUserData(vm, 1, &box));

        SQInteger size = box->getSizeBytes();

        sq_pushinteger(vm, size);

        return 1;
    }

    SQInteger TextureBoxUserData::_checkPointerOverflow(HSQUIRRELVM vm, TextureBoxWrapper* wrapper, size_t size){
        if(wrapper->seekCount + size > wrapper->getSizeBytes()) return sq_throwerror(vm, "Seek has exceeded box size in bytes.");
        return 0;
    }
    SQInteger TextureBoxUserData::_checkPointerOverflowFromStart(HSQUIRRELVM vm, TextureBoxWrapper* wrapper, size_t size){
        if(size > wrapper->getSizeBytes()) return sq_throwerror(vm, "Seek has exceeded box size in bytes.");
        return 0;
    }

    SQInteger TextureBoxUserData::tell(HSQUIRRELVM vm){
        TextureBoxWrapper* box;
        SCRIPT_ASSERT_RESULT(_readTextureBoxFromUserData(vm, 1, &box));

        sq_pushinteger(vm, static_cast<SQInteger>(box->seekCount));

        return 1;
    }

    SQInteger TextureBoxUserData::seek(HSQUIRRELVM vm){
        TextureBoxWrapper* box;
        SCRIPT_ASSERT_RESULT(_readTextureBoxFromUserData(vm, 1, &box));

        SQInteger pos;
        sq_getinteger(vm, 2, &pos);
        if(pos < 0) return sq_throwerror(vm, "Seek value cannot be negative.");
        size_t p = static_cast<size_t>(pos);

        SQInteger result = _checkPointerOverflowFromStart(vm, box, p);
        if(SQ_FAILED(result)) return result;

        box->seekPos = static_cast<void*>(static_cast<char*>(box->data) + p);
        box->seekCount = p;

        return 0;
    }

    SQInteger TextureBoxUserData::getColourAt(HSQUIRRELVM vm){
        TextureBoxWrapper* box;
        SCRIPT_ASSERT_RESULT(_readTextureBoxFromUserData(vm, 1, &box));

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

    SQInteger TextureBoxUserData::Write(HSQUIRRELVM vm, TextureBoxWrapper* wrapper, void* buffer, size_t size){
        SQInteger result = _checkPointerOverflow(vm, wrapper, size);
        if(SQ_FAILED(result)) return result;
        memcpy(wrapper->seekPos, buffer, size);
        wrapper->seekPos = static_cast<void*>(static_cast<char*>(wrapper->seekPos) + size);
        wrapper->seekCount += size;
        return 0;
    }
    SQInteger TextureBoxUserData::writeVal(HSQUIRRELVM v){
        TextureBoxWrapper* box;
        SCRIPT_ASSERT_RESULT(_readTextureBoxFromUserData(v, 1, &box));

        SQInteger format, ti;
        SQFloat tf;
        sq_getinteger(v, 3, &format);
        switch(format) {
            case 'l': {
                SQInteger i;
                sq_getinteger(v, 2, &ti);
                i = ti;
                return Write(v, box, &i, sizeof(SQInteger));
                      }
                break;
            case 'i': {
                SQInt32 i;
                sq_getinteger(v, 2, &ti);
                i = (SQInt32)ti;
                return Write(v, box, &i, sizeof(SQInt32));
                      }
                break;
            case 's': {
                short s;
                sq_getinteger(v, 2, &ti);
                s = (short)ti;
                return Write(v, box, &s, sizeof(short));
                      }
                break;
            case 'w': {
                unsigned short w;
                sq_getinteger(v, 2, &ti);
                w = (unsigned short)ti;
                return Write(v, box, &w, sizeof(unsigned short));
                      }
                break;
            case 'c': {
                char c;
                sq_getinteger(v, 2, &ti);
                c = (char)ti;
                return Write(v, box, &c, sizeof(char));
                          }
                break;
            case 'b': {
                unsigned char b;
                sq_getinteger(v, 2, &ti);
                b = (unsigned char)ti;
                return Write(v, box, &b, sizeof(unsigned char));
                      }
                break;
            case 'f': {
                float f;
                sq_getfloat(v, 2, &tf);
                f = (float)tf;
                return Write(v, box, &f, sizeof(float));
                      }
                break;
            case 'd': {
                double d;
                sq_getfloat(v, 2, &tf);
                d = tf;
                return Write(v, box, &d, sizeof(double));
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
        ScriptUtils::addFunction(vm, writeVal, "writen", 3, ".ni");
        ScriptUtils::addFunction(vm, seek, "seek", 2, ".i");
        ScriptUtils::addFunction(vm, tell, "tell");

        sq_resetobject(&TextureBoxDelegateTableObject);
        sq_getstackobj(vm, -1, &TextureBoxDelegateTableObject);
        sq_addref(vm, &TextureBoxDelegateTableObject);
        sq_pop(vm, 1);
    }
}
