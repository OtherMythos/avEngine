#include "SceneNamespace.h"

#include "OgreSceneManager.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/SceneNodeUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/MovableObjectUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    Ogre::SceneManager* SceneNamespace::_scene = 0;

    SQInteger SceneNamespace::getRootSceneNode(HSQUIRRELVM vm){
        //TODO in future I'm going to limit the root node for scripts to be something else.
        SceneNodeUserData::sceneNodeToUserData(vm, _scene->getRootSceneNode());

        return 1;
    }

    SQInteger SceneNamespace::createItem(HSQUIRRELVM vm){
        const SQChar *meshPath;
        sq_getstring(vm, 2, &meshPath);

        SQInteger size = sq_gettop(vm);
        Ogre::SceneMemoryMgrTypes targetType = Ogre::SCENE_DYNAMIC;
        if(size == 3){
            SQInteger sceneNodeType = 0;
            sq_getinteger(vm, 3, &sceneNodeType);
            targetType = static_cast<Ogre::SceneMemoryMgrTypes>(sceneNodeType);
        }

        Ogre::Item* item = _scene->createItem(meshPath, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, targetType);

        MovableObjectUserData::movableObjectToUserData(vm, (Ogre::MovableObject*)item, MovableObjectUserData::MovableObjectType::Item);

        return 1;
    }

    SQInteger SceneNamespace::createLight(HSQUIRRELVM vm){
        Ogre::Light* light = _scene->createLight();

        MovableObjectUserData::movableObjectToUserData(vm, (Ogre::MovableObject*)light, MovableObjectUserData::MovableObjectType::Light);

        return 1;
    }

    /**SQNamespace
    @name _scene
    @desc A namespace allowing access to the scene.
    */
    void SceneNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name getRootSceneNode
        @desc Obtain a handle to the root scene node.
        @returns A sceneNode handle.
        */
        ScriptUtils::addFunction(vm, getRootSceneNode, "getRootSceneNode");
        /**SQFunction
        @name createItem
        @desc Create an item, containing a mesh.
        @param1 A string containing a mesh name.
        @param2 Either _SCENE_STATIC or _SCENE_DYNAMIC. Defaults to _SCENE_DYNAMIC if not provided.
        @returns A movableObject userData.
        */
        ScriptUtils::addFunction(vm, createItem, "createItem", -2, ".si");

        ScriptUtils::addFunction(vm, createLight, "createLight");
    }

}
