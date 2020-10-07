#include "SceneNamespace.h"

#include "OgreSceneManager.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/SceneNodeUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/MovableObjectUserData.h"
#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/RayUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "OgreItem.h"
#include "OgreLight.h"

namespace AV{

    Ogre::SceneManager* SceneNamespace::_scene = 0;

    static Ogre::MovableObject::Listener itemListener;
    static Ogre::MovableObject::Listener lightListener;

    MovableObjectType SceneNamespace::determineTypeFromMovableObject(const Ogre::MovableObject* obj){
        Ogre::MovableObject::Listener* listener = obj->getListener();
        if(listener == &itemListener) return MovableObjectType::Item;
        else if(listener == &lightListener) return MovableObjectType::Light;
        else{
            assert(false); //I don't want to reach this point.
            return MovableObjectType::Any;
        }
    }

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

        Ogre::Item* item = 0;
        try{
            item = _scene->createItem(meshPath, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, targetType);
        }catch(Ogre::Exception& e){
            std::string s("Error reading mesh: ");
            s += e.getDescription();
            return sq_throwerror(vm, s.c_str());
        }
        item->setListener(&itemListener);

        MovableObjectUserData::movableObjectToUserData(vm, (Ogre::MovableObject*)item, MovableObjectType::Item);

        return 1;
    }

    SQInteger SceneNamespace::createLight(HSQUIRRELVM vm){
        Ogre::Light* light = _scene->createLight();
        light->setListener(&lightListener);

        MovableObjectUserData::movableObjectToUserData(vm, (Ogre::MovableObject*)light, MovableObjectType::Light);

        return 1;
    }

    SQInteger SceneNamespace::testRayForSlot(HSQUIRRELVM vm){
        Ogre::Ray targetRay;
        SCRIPT_CHECK_RESULT(RayUserData::readRayFromUserData(vm, 2, &targetRay));
        //Optimisation This could be shared so it's not re-created and destroyed each call.
        Ogre::RaySceneQuery* sceneQuery = _scene->createRayQuery(targetRay);
        const Ogre::RaySceneQueryResult& result = sceneQuery->execute();

        if(result.empty()){
            sq_pushnull(vm);
            _scene->destroyQuery(sceneQuery);
            return 1;
        }

        bool pushed = false;
        int lowestIdx = -1;
        Ogre::Real lowestDistance = 1000000;
        for(int i = 0; i < result.size(); i++){
            const Ogre::RaySceneQueryResultEntry& e = result[i];
            if(e.distance <= 5) continue;
            if(e.distance < lowestDistance){
                lowestDistance = e.distance;
                lowestIdx = i;
            }
        }

        if(lowestIdx >= 0){
            const Ogre::Vector3 foundPoint = targetRay.getPoint(result[lowestIdx].distance);

            SlotPositionClass::createNewInstance(vm, SlotPosition(foundPoint));
        }else{
            sq_pushnull(vm);
        }

        _scene->destroyQuery(sceneQuery);
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
        /**SQFunction
        @name testRayForSlot
        @desc Perform a ray test on the Ogre scene, finding a slot position of the nearest collision.
        @param1:Ray: A ray object to test with.
        @returns A SlotPosition if a collision was found. Null if nothing was found.
        */
        ScriptUtils::addFunction(vm, testRayForSlot, "testRayForSlot", 2, ".u");
    }

}
