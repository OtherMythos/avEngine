#include "MeshClass.h"

#include "Serialisation/SaveHandle.h"
#include "SlotPositionClass.h"
#include "PhysicsClasses/PhysicsRigidBodyClass.h"

#include "World/WorldSingleton.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"

#include "Ogre.h"

namespace AV{
    SQObject MeshClass::classObject;
    ScriptDataPacker<OgreMeshManager::OgreMeshPtr> MeshClass::mMeshData;
    std::map<Ogre::SceneNode*, PhysicsBodyConstructor::RigidBodyPtr> MeshClass::mAttachedMeshes;

    void MeshClass::setupClass(HSQUIRRELVM vm){
        sq_newclass(vm, 0);

        sq_pushstring(vm, _SC("setPosition"), -1);
        sq_newclosure(vm, setMeshPosition, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("getPosition"), -1);
        sq_newclosure(vm, getMeshPosition, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("getPositionRaw"), -1);
        sq_newclosure(vm, getMeshPositionRaw, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("setScale"), -1);
        sq_newclosure(vm, setScale, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("setOrientation"), -1);
        sq_newclosure(vm, setOrientation, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("attachRigidBody"), -1);
        sq_newclosure(vm, attachRigidBody, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("detachRigidBody"), -1);
        sq_newclosure(vm, detachRigidBody, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("_cmp"), -1);
        sq_newclosure(vm, meshCompare, 0);
        sq_newslot(vm, -3, false);

        sq_resetobject(&classObject);
        sq_getstackobj(vm, -1, &classObject);
        sq_addref(vm, &classObject);
        sq_pop(vm, 1);
    }

    SQInteger MeshClass::meshCompare(HSQUIRRELVM vm){
        SQObjectType pT = sq_gettype(vm, -1);
        SQObjectType qT = sq_gettype(vm, -2);
        if(pT != OT_INSTANCE || qT != OT_INSTANCE){
            sq_pushbool(vm, false);
            return 1;
        }

        SQUserPointer p;
        SQUserPointer q;
        sq_getinstanceup(vm, -1, &p, 0);
        sq_getinstanceup(vm, -2, &q, 0);

        sq_pushbool(vm, p == q);
        return 1;
    }

    SQInteger MeshClass::sqMeshReleaseHook(SQUserPointer p, SQInteger size){
        Ogre::SceneNode* node = mMeshData.getEntry(p).get();
        //I need to take a copy of the shared pointer here so as not to risk its destruction while I'm working with it.

        bool inWorld = _meshAttached(node);
        if(inWorld){
            PhysicsBodyConstructor::RigidBodyPtr bdy = mAttachedMeshes[node];
            mAttachedMeshes.erase(node);
            //If an element was erased, i.e this mesh had a rigid body attached to it.

            World* w = WorldSingleton::getWorld();
            if(w){
                w->getPhysicsManager()->getDynamicsWorld()->detachMeshFromBody(bdy);
            }
        }

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

    SQInteger MeshClass::getMeshPositionRaw(HSQUIRRELVM vm){
        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, -1);

        Ogre::Vector3 vec = mesh->getPosition();

        sq_newarray(vm, 3);
        sq_pushfloat(vm, vec.z);
        sq_pushfloat(vm, vec.y);
        sq_pushfloat(vm, vec.x);
        sq_arrayinsert(vm, -4, 0);
        sq_arrayinsert(vm, -3, 1);
        sq_arrayinsert(vm, -2, 2);

        return 1;
    }

    SQInteger MeshClass::getMeshPosition(HSQUIRRELVM vm){
        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, -1);

        Ogre::Vector3 pos = mesh->getPosition();
        SlotPosition slotPos(pos);

        SlotPositionClass::instanceFromSlotPosition(vm, slotPos);

        return 1;
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

    SQInteger MeshClass::attachRigidBody(HSQUIRRELVM vm){
        //TODO technically the rigid bodies can be created and used separate from the world.
        //Should this instead work that meshes can have a body attached even if a world doesn't exist?
        World* w = WorldSingleton::getWorld();
        if(!w) return 0;

        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, -2);
        Ogre::SceneNode* node = mesh.get();
        if(_meshAttached(node)) return 0;

        PhysicsBodyConstructor::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);

        if(!w->getPhysicsManager()->getDynamicsWorld()->attachMeshToBody(body, mesh.get())) return 0;

        //Keep a reference to the body, so it's not destroyed.
        mAttachedMeshes.insert({node, body});

        return 0;
    }

    SQInteger MeshClass::detachRigidBody(HSQUIRRELVM vm){
        World* w = WorldSingleton::getWorld();

        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, -1);
        Ogre::SceneNode* node = mesh.get();
        if(!_meshAttached(node)) return 0;

        if(w){
            w->getPhysicsManager()->getDynamicsWorld()->detachMeshFromBody(mAttachedMeshes[node]);
        }
        mAttachedMeshes.erase(node);

        return 0;
    }

    bool MeshClass::_meshAttached(Ogre::SceneNode* mesh){
        return mAttachedMeshes.find(mesh) != mAttachedMeshes.end();
    }

}
