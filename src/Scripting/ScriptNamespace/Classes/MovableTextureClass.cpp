#include "MovableTextureClass.h"

#include "System/BaseSingleton.h"

#include "Ogre.h"

#include "Logger/Log.h"

#include "../ScriptUtils.h"

namespace AV{
    SQObject MovableTextureClass::classObject;
    ScriptDataPacker<MovableTexturePtr> MovableTextureClass::mTextures;

    void MovableTextureClass::setupClass(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("MovableTexture"), -1);
        sq_newclass(vm, 0);


        sq_pushstring(vm, _SC("constructor"), -1);
        sq_newclosure(vm, movableTextureConstructor, 0);
        sq_newslot(vm, -3, false);


        sq_newslot(vm, -3, false);
    }

    SQInteger MovableTextureClass::movableTextureConstructor(HSQUIRRELVM vm){
        SQInteger nargs = sq_gettop(vm);
        ScriptUtils::_debugStack(vm);

        const SQChar *texturePath;
        const SQChar *textureGroup = "General"; //Default to General unless specified.

        SQInteger popAmount = 0;
        if(nargs < 3){ //Just the texture
            sq_getstring(vm, -1, &texturePath);
            popAmount = 1;
        }
        if(nargs == 3){ //texture and group
            sq_getstring(vm, -1, &textureGroup);
            sq_getstring(vm, -2, &texturePath);
            popAmount = 2;
        }
        sq_pop(vm, popAmount);

        assert(sq_gettype(vm, sq_gettop(vm)) == OT_INSTANCE); //The top of the stack should be the instance just created.

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

}
