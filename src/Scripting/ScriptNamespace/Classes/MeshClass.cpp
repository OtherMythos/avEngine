#include "MeshClass.h"

#include "Serialisation/SaveHandle.h"
#include "SlotPositionClass.h"

#include "Ogre.h"

namespace AV{
    SQObject MeshClass::classObject;
    ScriptDataPacker<OgreMeshManager::OgreMeshPtr> MeshClass::mMeshData;

    void MeshClass::setupClass(HSQUIRRELVM vm){
        sq_newclass(vm, 0);

        sq_pushstring(vm, _SC("setPosition"), -1);
        sq_newclosure(vm, setMeshPosition, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("setScale"), -1);
        sq_newclosure(vm, setScale, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("setOrientation"), -1);
        sq_newclosure(vm, setOrientation, 0);
        sq_newslot(vm, -3, false);

        sq_resetobject(&classObject);
        sq_getstackobj(vm, -1, &classObject);
        sq_addref(vm, &classObject);
        sq_pop(vm, 1);
    }

    SQInteger MeshClass::sqMeshReleaseHook(SQUserPointer p, SQInteger size){
        mMeshData.getEntry(p).reset();

        mMeshData.removeEntry(p);

        return 0;
    }

    void MeshClass::MeshPtrToInstance(HSQUIRRELVM vm, OgreMeshManager::OgreMeshPtr mesh){
        sq_pushobject(vm, classObject);
        sq_createinstance(vm, -1);

        void* id = mMeshData.storeEntry(mesh);
        sq_setinstanceup(vm, -1, (SQUserPointer*)id);

        sq_setreleasehook(vm, -1, sqMeshReleaseHook);
    }

    OgreMeshManager::OgreMeshPtr MeshClass::instanceToMeshPtr(HSQUIRRELVM vm, SQInteger index){
        SQUserPointer p;
        sq_getinstanceup(vm, index, &p, 0);

        return mMeshData.getEntry(p);
    }

    SQInteger MeshClass::setMeshPosition(HSQUIRRELVM vm){
        SlotPosition pos = SlotPositionClass::getSlotFromInstance(vm, -1);

        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, -2);

        mesh->setPosition(pos.toOgre());

        return 0;
    }

    SQInteger MeshClass::setScale(HSQUIRRELVM vm){
        SQFloat x, y, z;
        sq_getfloat(vm, -1, &z);
        sq_getfloat(vm, -2, &y);
        sq_getfloat(vm, -3, &x);

        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, -4);

        Ogre::Vector3 scale(x, y, z);
        mesh->setScale(scale);

        return 0;
    }

    SQInteger MeshClass::setOrientation(HSQUIRRELVM vm){
        SQFloat x, y, z, w;
        sq_getfloat(vm, -1, &w);
        sq_getfloat(vm, -2, &z);
        sq_getfloat(vm, -3, &y);
        sq_getfloat(vm, -4, &x);

        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, -5);

        mesh->setOrientation(Ogre::Quaternion(x, y, z, w));

        return 0;
    }
}
