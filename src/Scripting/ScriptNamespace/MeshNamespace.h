#pragma once

#include "ScriptNamespace.h"

namespace Ogre{
    class SceneManager;
    class SceneNode;
}

namespace AV{
    class ScriptManager;

    struct squirrelOgreMeshData : public squirrelDataType{
        squirrelOgreMeshData(Ogre::SceneNode* node) : squirrelDataType(AV_SQ_DATA_TYPE_MESH), node(node) {};
        ~squirrelOgreMeshData(){

        }
        /**
         A pointer to the stored scene node. This should contain the ogre mesh.
         */
        Ogre::SceneNode *node = 0;
    };

    class MeshNamespace : public ScriptNamespace{
        friend ScriptManager;
    public:
        MeshNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);

    private:
        static Ogre::SceneManager* _sceneManager;

        /**
         Create an ogre mesh from an x, y, z and push the result onto the stack as a user data.
         */
        static SQInteger createMesh(HSQUIRRELVM vm);
        /**
         Destroy a mesh user data at the top of the stack.
         */
        static SQInteger destroyMesh(HSQUIRRELVM vm);
        /**
         Set the x,y,z position of a user data meshs at the top of the stack.
         */
        static SQInteger setMeshPosition(HSQUIRRELVM vm);
        /**
         Maintenance function called by squirrel when an OgreMesh user data is destroyed.
         */
        static SQInteger sqOgreMeshDataReleaseHook(SQUserPointer p,SQInteger size);
    };
}
