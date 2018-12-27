#include "MeshNamespace.h"

#include "Logger/Log.h"

#include "OgreSceneManager.h"
#include "OgreVector3.h"

namespace AV{
    Ogre::SceneManager* MeshNamespace::_sceneManager = 0;
    
    SQInteger MeshNamespace::createMesh(HSQUIRRELVM vm){
        const SQChar *meshPath;
        sq_getstring(vm, -1, &meshPath);
        
        Ogre::SceneNode *node = _sceneManager->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_DYNAMIC);
        Ogre::Item *item = _sceneManager->createItem(meshPath, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_DYNAMIC);
        node->attachObject((Ogre::MovableObject*)item);
        
        void* pointer = (void*)sq_newuserdata(vm, sizeof(squirrelOgreMeshData));
        new (pointer) squirrelOgreMeshData(node);

        return 1;
    }
    
    SQInteger MeshNamespace::destroyMesh(HSQUIRRELVM vm){
        SQUserPointer pointer;
        sq_getuserdata(vm, -1, &pointer, NULL);
        
        squirrelOgreMeshData *data = static_cast<squirrelOgreMeshData*>(pointer);
        _sceneManager->destroySceneNode(data->node);

        return 0;
    }
    
    SQInteger MeshNamespace::setMeshPosition(HSQUIRRELVM vm){
        SQFloat x, y, z;
        sq_getfloat(vm, -1, &z);
        sq_getfloat(vm, -2, &y);
        sq_getfloat(vm, -3, &x);
        
        SQUserPointer pointer;
        sq_getuserdata(vm, -4, &pointer, NULL);
        
        squirrelOgreMeshData *data = static_cast<squirrelOgreMeshData*>(pointer);
        
        data->node->setPosition(x, y, z);
        
        return 0;
    }
    
    void MeshNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, createMesh, "createMesh", 2, ".s");
        _addFunction(vm, destroyMesh, "destroyMesh", 2, ".u");
        _addFunction(vm, setMeshPosition, "setPosition", 5, ".unnn");
    }
}
