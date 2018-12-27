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
        Ogre::SceneNode *node = 0;
    };
    
    class MeshNamespace : public ScriptNamespace{
        friend ScriptManager;
    public:
        MeshNamespace() {};
        
        void setupNamespace(HSQUIRRELVM vm);
        
    private:
        static Ogre::SceneManager* _sceneManager;
        
        static SQInteger createMesh(HSQUIRRELVM vm);
        static SQInteger destroyMesh(HSQUIRRELVM vm);
        static SQInteger setMeshPosition(HSQUIRRELVM vm);
    };
}
