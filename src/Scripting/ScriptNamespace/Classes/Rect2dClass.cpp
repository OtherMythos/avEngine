#include "Rect2dClass.h"

#include "System/BaseSingleton.h"
#include "Gui/Texture2d/MovableTexture.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "Ogre/Hlms/DatablockUserData.h"

namespace AV{
    SQObject Rect2dClass::classObject;
    ScriptDataPacker<Rect2dPtr> Rect2dClass::mRects;

    void Rect2dClass::setupClass(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("Rect2d"), -1);
        sq_newclass(vm, 0);

        ScriptUtils::addFunction(vm, movableTextureConstructor, "constructor");
        ScriptUtils::addFunction(vm, setRectPosition, "setPosition");
        ScriptUtils::addFunction(vm, setRectWidth, "setWidth");
        ScriptUtils::addFunction(vm, setRectHeight, "setHeight");
        ScriptUtils::addFunction(vm, setRectSize, "setSize");
        ScriptUtils::addFunction(vm, setRectVisible, "setVisible", 2, ".b");
        ScriptUtils::addFunction(vm, setLayer, "setLayer");
        ScriptUtils::addFunction(vm, getLayer, "getLayer");
        ScriptUtils::addFunction(vm, setDatablock, "setDatablock", 2, ".u");

        sq_newslot(vm, -3, false);
    }

    SQInteger Rect2dClass::movableTextureConstructor(HSQUIRRELVM vm){
        Rect2dPtr rec = BaseSingleton::getRect2dManager()->createRect2d();
        void* id = mRects.storeEntry(rec);

        sq_setinstanceup(vm, -1, (SQUserPointer*)id);
        sq_setreleasehook(vm, -1, sqRectReleaseHook);

        return 0;
    }

    SQInteger Rect2dClass::sqRectReleaseHook(SQUserPointer p, SQInteger size){
        mRects.getEntry(p).reset();
        mRects.removeEntry(p);

        return 0;
    }

    SQInteger Rect2dClass::setLayer(HSQUIRRELVM vm){
        SQInteger layerId;
        sq_getinteger(vm, -1, &layerId);

        SQUserPointer p;
        sq_getinstanceup(vm, -2, &p, 0);
        Rect2dPtr rec = mRects.getEntry(p);

        BaseSingleton::getRect2dManager()->setRectLayer(rec, layerId);

        return 0;
    }

    SQInteger Rect2dClass::getLayer(HSQUIRRELVM vm){
        SQUserPointer p;
        sq_getinstanceup(vm, -1, &p, 0);
        Rect2dPtr rec = mRects.getEntry(p);

        sq_pushinteger(vm, rec->getLayer());

        return 1;
    }

    SQInteger Rect2dClass::setRectPosition(HSQUIRRELVM vm){
        SQFloat x, y;
        SQUserPointer p;

        sq_getfloat(vm, -1, &y);
        sq_getfloat(vm, -2, &x);
        sq_getinstanceup(vm, -3, &p, 0);

        Rect2dPtr rec = mRects.getEntry(p);
        rec->setPosition(x, y);

        return 0;
    }

    SQInteger Rect2dClass::setRectWidth(HSQUIRRELVM vm){
        SQFloat w;
        SQUserPointer p;

        sq_getfloat(vm, -1, &w);
        sq_getinstanceup(vm, -2, &p, 0);

        Rect2dPtr rec = mRects.getEntry(p);
        rec->setWidth(w);

        return 0;
    }

    SQInteger Rect2dClass::setRectHeight(HSQUIRRELVM vm){
        SQFloat h;
        SQUserPointer p;

        sq_getfloat(vm, -1, &h);
        sq_getinstanceup(vm, -2, &p, 0);

        Rect2dPtr rec = mRects.getEntry(p);
        rec->setHeight(h);

        return 0;
    }

    SQInteger Rect2dClass::setRectSize(HSQUIRRELVM vm){
        SQFloat w, h;
        SQUserPointer p;

        sq_getfloat(vm, -1, &h);
        sq_getfloat(vm, -2, &w);
        sq_getinstanceup(vm, -3, &p, 0);

        Rect2dPtr rec = mRects.getEntry(p);
        rec->setSize(w, h);

        return 0;
    }

    SQInteger Rect2dClass::setRectVisible(HSQUIRRELVM vm){
        SQBool visible;
        SQUserPointer p;

        sq_getbool(vm, -1, &visible);
        sq_getinstanceup(vm, -2, &p, 0);

        Rect2dPtr rec = mRects.getEntry(p);
        rec->setVisible(visible);

        return 0;
    }

    SQInteger Rect2dClass::setDatablock(HSQUIRRELVM vm){
        SQUserPointer p;
        Ogre::HlmsDatablock* db = 0;
        DatablockUserData::getPtrFromUserData(vm, -1, &db);

        sq_getinstanceup(vm, -2, &p, 0);

        Rect2dPtr rec = mRects.getEntry(p);
        rec->setDatablock(db);

        return 0;
    }

}
