#include "SceneNamespace.h"

#include "OgreSceneManager.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/SceneNodeUserData.h"

namespace AV{

    Ogre::SceneManager* SceneNamespace::_scene = 0;

    SQInteger SceneNamespace::getRootSceneNode(HSQUIRRELVM vm){
        //TODO in future I'm going to limit the root node for scripts to be something else.
        SceneNodeUserData::sceneNodeToUserData(vm, _scene->getRootSceneNode());

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
    }

}
