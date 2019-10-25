#include "MovableTextureClass.h"

#include "System/BaseSingleton.h"
#include "Gui/Texture2d/MovableTexture.h"

namespace AV{
    SQObject MovableTextureClass::classObject;
    ScriptDataPacker<MovableTexturePtr> MovableTextureClass::mTextures;

    void MovableTextureClass::setupClass(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("MovableTexture"), -1);
        sq_newclass(vm, 0);


        sq_pushstring(vm, _SC("constructor"), -1);
        sq_newclosure(vm, movableTextureConstructor, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("setPosition"), -1);
        sq_newclosure(vm, setTexturePosition, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("setWidth"), -1);
        sq_newclosure(vm, setTextureWidth, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("setHeight"), -1);
        sq_newclosure(vm, setTextureHeight, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("setSize"), -1);
        sq_newclosure(vm, setTextureSize, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("setTexture"), -1);
        sq_newclosure(vm, setTexture, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("setSectionScale"), -1);
        sq_newclosure(vm, setSectionScale, 0);
        sq_newslot(vm, -3, false);


        sq_newslot(vm, -3, false);
    }

    void MovableTextureClass::_getTextureStrings(HSQUIRRELVM vm, const SQChar** name, const SQChar** group){

        SQInteger nargs = sq_gettop(vm);
        const SQChar *textureName;
        const SQChar *textureGroup = "General"; //Default to General unless specified.

        SQInteger popAmount = 0;
        if(nargs < 3){ //Just the texture
            sq_getstring(vm, -1, &textureName);
            popAmount = 1;
        }
        if(nargs == 3){ //texture and group
            sq_getstring(vm, -1, &textureGroup);
            sq_getstring(vm, -2, &textureName);
            popAmount = 2;
        }
        sq_pop(vm, popAmount);

        assert(sq_gettype(vm, sq_gettop(vm)) == OT_INSTANCE); //The top of the stack should be the instance just created.

        *name = textureName;
        *group = textureGroup;
    }

    SQInteger MovableTextureClass::movableTextureConstructor(HSQUIRRELVM vm){
        const SQChar *texturePath;
        const SQChar *textureGroup;
        _getTextureStrings(vm, &texturePath, &textureGroup);

        MovableTexturePtr tex = BaseSingleton::getMovableTextureManager()->createTexture(texturePath, textureGroup);
        void* id = mTextures.storeEntry(tex);

        sq_setinstanceup(vm, -1, (SQUserPointer*)id);
        sq_setreleasehook(vm, -1, sqTextureReleaseHook);

        return 0;
    }

    SQInteger MovableTextureClass::sqTextureReleaseHook(SQUserPointer p, SQInteger size){
        //Don't need to do much here as I'm wrapping around a shared pointer anyway.
        mTextures.getEntry(p).reset();
        mTextures.removeEntry(p);

        return 0;
    }

    SQInteger MovableTextureClass::setTexture(HSQUIRRELVM vm){
        const SQChar *texturePath;
        const SQChar *textureGroup;
        _getTextureStrings(vm, &texturePath, &textureGroup);

        SQUserPointer p;
        sq_getinstanceup(vm, -1, &p, 0);

        MovableTexturePtr tex = mTextures.getEntry(p);
        tex->setTexture(texturePath, textureGroup);

        return 0;
    }

    SQInteger MovableTextureClass::setTexturePosition(HSQUIRRELVM vm){
        SQFloat x, y;
        SQUserPointer p;

        sq_getfloat(vm, -1, &y);
        sq_getfloat(vm, -2, &x);
        sq_getinstanceup(vm, -3, &p, 0);

        MovableTexturePtr tex = mTextures.getEntry(p);
        tex->setPosition(x, y);

        return 0;
    }

    SQInteger MovableTextureClass::setTextureWidth(HSQUIRRELVM vm){
        SQFloat w;
        SQUserPointer p;

        sq_getfloat(vm, -1, &w);
        sq_getinstanceup(vm, -2, &p, 0);

        MovableTexturePtr tex = mTextures.getEntry(p);
        tex->setWidth(w);

        return 0;
    }

    SQInteger MovableTextureClass::setTextureHeight(HSQUIRRELVM vm){
        SQFloat h;
        SQUserPointer p;

        sq_getfloat(vm, -1, &h);
        sq_getinstanceup(vm, -2, &p, 0);

        MovableTexturePtr tex = mTextures.getEntry(p);
        tex->setHeight(h);

        return 0;
    }

    SQInteger MovableTextureClass::setTextureSize(HSQUIRRELVM vm){
        SQFloat w, h;
        SQUserPointer p;

        sq_getfloat(vm, -1, &h);
        sq_getfloat(vm, -2, &w);
        sq_getinstanceup(vm, -3, &p, 0);

        MovableTexturePtr tex = mTextures.getEntry(p);
        tex->setSize(w, h);

        return 0;
    }

    SQInteger MovableTextureClass::setSectionScale(HSQUIRRELVM vm){
        SQFloat scaleX, scaleY, posX, posY;
        SQUserPointer p;

        sq_getfloat(vm, -1, &posY);
        sq_getfloat(vm, -2, &posX);
        sq_getfloat(vm, -3, &scaleY);
        sq_getfloat(vm, -4, &scaleX);
        sq_getinstanceup(vm, -5, &p, 0);

        MovableTexturePtr tex = mTextures.getEntry(p);
        tex->setSectionScale(scaleX, scaleY, posX, posY);

        return 0;
    }

}
