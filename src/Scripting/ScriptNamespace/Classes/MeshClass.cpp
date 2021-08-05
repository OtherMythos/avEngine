#include "MeshClass.h"

#include "Serialisation/SaveHandle.h"
#include "SlotPositionClass.h"
#include "PhysicsClasses/PhysicsRigidBodyClass.h"

#include "World/WorldSingleton.h"
#include "World/Physics/PhysicsManager.h"
#include "World/Physics/Worlds/DynamicsWorld.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Ogre/Hlms/DatablockUserData.h"
#include "OgreItem.h"

#include "Ogre.h"
#include "System/EngineFlags.h"

namespace AV{
    SQObject MeshClass::classObject;
    DataPacker<OgreMeshManager::OgreMeshPtr> MeshClass::mMeshData;
    std::map<Ogre::SceneNode*, PhysicsTypes::RigidBodyPtr> MeshClass::mAttachedMeshes;

    void MeshClass::setupClass(HSQUIRRELVM vm){
        sq_newclass(vm, 0);

        ScriptUtils::addFunction(vm, setMeshPosition, "setPosition");
        ScriptUtils::addFunction(vm, getMeshPosition, "getPosition");
        ScriptUtils::addFunction(vm, getMeshPositionRaw, "getPositionRaw");
        ScriptUtils::addFunction(vm, setScale, "setScale");
        ScriptUtils::addFunction(vm, setOrientation, "setOrientation");
        ScriptUtils::addFunction(vm, attachRigidBody, "attachRigidBody");
        ScriptUtils::addFunction(vm, detachRigidBody, "detachRigidBody");
        ScriptUtils::addFunction(vm, setDatablock, "setDatablock");
        ScriptUtils::addFunction(vm, meshCompare, "_cmp");

        sq_resetobject(&classObject);
        sq_getstackobj(vm, -1, &classObject);
        sq_addref(vm, &classObject);
        sq_pop(vm, 1);
    }

    SQInteger MeshClass::meshCompare(HSQUIRRELVM vm){
        SQObjectType pT = sq_gettype(vm, -1);
        SQObjectType qT = sq_gettype(vm, -2);
        if(pT != OT_INSTANCE || qT != OT_INSTANCE){
            sq_pushinteger(vm, 2);
            return 1;
        }

        SQUserPointer p;
        SQUserPointer q;
        sq_getinstanceup(vm, -1, &p, 0);
        sq_getinstanceup(vm, -2, &q, 0);

        //2 means not equal. Any number other than 0, 1, -2 seems to mean that infact.
        sq_pushinteger(vm, mMeshData.getEntry(p) == mMeshData.getEntry(q) ? 0 : 2);
        return 1;
    }

    SQInteger MeshClass::sqMeshReleaseHook(SQUserPointer p, SQInteger size){
        Ogre::SceneNode* node = mMeshData.getEntry(p).get();
        //I need to take a copy of the shared pointer here so as not to risk its destruction while I'm working with it.

        bool inWorld = _meshAttached(node);
        if(inWorld){
            //In order for the mesh to have been attached, physics must have existed. If it doesn't here then something has gone wrong.
            ASSERT_DYNAMIC_PHYSICS();
            PhysicsTypes::RigidBodyPtr bdy = mAttachedMeshes[node];
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
        CHECK_SCENE_CLEAN()
        SlotPosition pos;
        SCRIPT_CHECK_RESULT(SlotPositionClass::getSlotFromInstance(vm, -1, &pos));

        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, -2);

        Ogre::Vector3 absPos = pos.toOgre();
        mesh->setPosition(absPos);

        if(_meshAttached(mesh.get())){
            ASSERT_DYNAMIC_PHYSICS();
            //Also reposition the rigid body if one exists.
            World* w = WorldSingleton::getWorld();
            if(w){
                btVector3 btAbsPos(absPos.x, absPos.y, absPos.z);
                w->getPhysicsManager()->getDynamicsWorld()->setBodyPosition(mAttachedMeshes[mesh.get()], btAbsPos);
            }
        }


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

        SlotPositionClass::createNewInstance(vm, slotPos);

        return 1;
    }

    SQInteger MeshClass::setDatablock(HSQUIRRELVM vm){
        Ogre::HlmsDatablock* db = 0;
        SCRIPT_CHECK_RESULT(DatablockUserData::getPtrFromUserData(vm, -1, &db));

        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, -2);
        ((Ogre::Item*)(mesh->getAttachedObject(0)))->setDatablock(db);

        return 0;
    }

    SQInteger MeshClass::setScale(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
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
        CHECK_SCENE_CLEAN()
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

        CHECK_DYNAMIC_PHYSICS()
        SCRIPT_CHECK_WORLD();

        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, -2);
        Ogre::SceneNode* node = mesh.get();
        if(_meshAttached(node)) return sq_throwerror(vm, "Body already attached to mesh.");

        PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, -1);

        if(!world->getPhysicsManager()->getDynamicsWorld()->attachMeshToBody(body, mesh.get())) return sq_throwerror(vm, "Error attaching mesh");

        //Keep a reference to the body, so it's not destroyed.
        mAttachedMeshes.insert({node, body});

        return 0;
    }

    SQInteger MeshClass::detachRigidBody(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, -1);
        Ogre::SceneNode* node = mesh.get();
        if(!_meshAttached(node)) return sq_throwerror(vm, "No body attached.");

        ASSERT_DYNAMIC_PHYSICS();
        world->getPhysicsManager()->getDynamicsWorld()->detachMeshFromBody(mAttachedMeshes[node]);
        mAttachedMeshes.erase(node);

        return 0;
    }

    bool MeshClass::_meshAttached(Ogre::SceneNode* mesh){
        return mAttachedMeshes.find(mesh) != mAttachedMeshes.end();
    }

}
