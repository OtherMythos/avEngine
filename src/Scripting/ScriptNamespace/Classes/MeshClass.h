#pragma once

#include <squirrel.h>

#include "World/Support/OgreMeshManager.h"
#include "World/Physics/PhysicsBodyConstructor.h"
#include "Scripting/ScriptDataPacker.h"
#include <map>

namespace AV{

    /**
    A class to expose meshes to squirrel.
    */
    class MeshClass{
    public:
        MeshClass() = delete;

        static void setupClass(HSQUIRRELVM vm);

        static void MeshPtrToInstance(HSQUIRRELVM vm, OgreMeshManager::OgreMeshPtr mesh);
        static OgreMeshManager::OgreMeshPtr instanceToMeshPtr(HSQUIRRELVM vm, SQInteger index);

    private:
        static ScriptDataPacker<OgreMeshManager::OgreMeshPtr> mMeshData;
        static std::map<Ogre::SceneNode*, PhysicsTypes::RigidBodyPtr> mAttachedMeshes;
        static SQObject classObject;

        static SQInteger setMeshPosition(HSQUIRRELVM vm);
        static SQInteger getMeshPositionRaw(HSQUIRRELVM vm);
        static SQInteger getMeshPosition(HSQUIRRELVM vm);
        static SQInteger setScale(HSQUIRRELVM vm);
        static SQInteger setOrientation(HSQUIRRELVM vm);

        static SQInteger attachRigidBody(HSQUIRRELVM vm);
        static SQInteger detachRigidBody(HSQUIRRELVM vm);

        static SQInteger setDatablock(HSQUIRRELVM vm);

        static SQInteger meshCompare(HSQUIRRELVM vm);
        static SQInteger sqMeshReleaseHook(SQUserPointer p, SQInteger size);

        static inline bool _meshAttached(Ogre::SceneNode* shape);
    };
}
