#include "MeshClass.h"

#include "Serialisation/SaveHandle.h"
#include "SlotPositionClass.h"
#include "PhysicsClasses/PhysicsRigidBodyClass.h"
#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"
#include "Scripting/ScriptNamespace/Classes/QuaternionUserData.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"

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

        ScriptUtils::addFunction(vm, setMeshPosition, "setPosition", -2, ".n|unn");
        ScriptUtils::addFunction(vm, getMeshPosition, "getPosition");
        ScriptUtils::addFunction(vm, getMeshPositionVec3, "getPositionVec3");
        ScriptUtils::addFunction(vm, setScale, "setScale", -2, ".u|nnn");
        ScriptUtils::addFunction(vm, getScale, "getScale");
        ScriptUtils::addFunction(vm, setOrientation, "setOrientation", 2, ".u");
        ScriptUtils::addFunction(vm, getOrientation, "getOrientation");
        ScriptUtils::addFunction(vm, attachRigidBody, "attachRigidBody", 2, ".x");
        ScriptUtils::addFunction(vm, detachRigidBody, "detachRigidBody");
        ScriptUtils::addFunction(vm, setDatablock, "setDatablock", 2, ".u");
        ScriptUtils::addFunction(vm, meshCompare, "_cmp");
        ScriptUtils::addFunction(vm, setRenderQueueGroup, "setRenderQueueGroup", 2, ".i");

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
        sq_getinstanceup(vm, -1, &p, 0, false);
        sq_getinstanceup(vm, -2, &q, 0, false);

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
        sq_getinstanceup(vm, index, &p, 0, false);

        return mMeshData.getEntry(p);
    }

    SQInteger MeshClass::setMeshPosition(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
        Ogre::Vector3 absPos;
        SQInteger result = ScriptGetterUtils::vector3Read(vm, &absPos);
        if(result != 0) return result;

        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, 1);

        mesh->setPosition(absPos);

        if(_meshAttached(mesh.get())){
            ASSERT_DYNAMIC_PHYSICS();
            //Also reposition the rigid body if one exists.
            World* w = WorldSingleton::getWorld();
            if(w){
                w->getPhysicsManager()->getDynamicsWorld()->setBodyPosition(mAttachedMeshes[mesh.get()], OGRE_TO_BULLET(absPos));
            }
        }


        return 0;
    }

    SQInteger MeshClass::getMeshPositionVec3(HSQUIRRELVM vm){
        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, 1);

        Ogre::Vector3 pos = mesh->getPosition();
        Vector3UserData::vector3ToUserData(vm, pos);

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
        SCRIPT_CHECK_RESULT(DatablockUserData::getPtrFromUserData(vm, 2, &db));

        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, 1);
        ((Ogre::Item*)(mesh->getAttachedObject(0)))->setDatablock(db);

        return 0;
    }

    SQInteger MeshClass::setScale(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
        Ogre::Vector3 scale;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &scale));

        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, 1);

        mesh->setScale(scale);

        return 0;
    }

    SQInteger MeshClass::getScale(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, 1);

        Ogre::Vector3 vec = mesh->getScale();
        Vector3UserData::vector3ToUserData(vm, vec);

        return 1;
    }

    SQInteger MeshClass::setOrientation(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
        Ogre::Quaternion outQuat;
        SCRIPT_CHECK_RESULT(QuaternionUserData::readQuaternionFromUserData(vm, 2, &outQuat));

        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, 1);

        mesh->setOrientation(outQuat);

        return 0;
    }

    SQInteger MeshClass::getOrientation(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, 1);

        const Ogre::Quaternion orientation = mesh->getOrientation();
        QuaternionUserData::quaternionToUserData(vm, orientation);

        return 1;
    }

    SQInteger MeshClass::attachRigidBody(HSQUIRRELVM vm){
        //TODO technically the rigid bodies can be created and used separate from the world.
        //Should this instead work that meshes can have a body attached even if a world doesn't exist?

        CHECK_DYNAMIC_PHYSICS()
        SCRIPT_CHECK_WORLD();

        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, 1);
        Ogre::SceneNode* node = mesh.get();
        if(_meshAttached(node)) return sq_throwerror(vm, "Body already attached to mesh.");

        PhysicsTypes::RigidBodyPtr body = PhysicsRigidBodyClass::getRigidBodyFromInstance(vm, 2);

        if(!world->getPhysicsManager()->getDynamicsWorld()->attachMeshToBody(body, mesh.get())) return sq_throwerror(vm, "Error attaching mesh");

        //Keep a reference to the body, so it's not destroyed.
        mAttachedMeshes.insert({node, body});

        return 0;
    }

    SQInteger MeshClass::detachRigidBody(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, 1);
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


    SQInteger MeshClass::setRenderQueueGroup(HSQUIRRELVM vm){
        OgreMeshManager::OgreMeshPtr mesh = instanceToMeshPtr(vm, 1);

        SQInteger renderQueue;
        sq_getinteger(vm, 2, &renderQueue);

        if(renderQueue < 0 || renderQueue > 100) return sq_throwerror(vm, "Render queue must be in range 0-100");
        uint8 target = static_cast<uint8>(renderQueue);

        ((Ogre::Item*)(mesh->getAttachedObject(0)))->setRenderQueueGroup(target);

        return 0;
    }

}
