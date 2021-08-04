#include "SceneNamespace.h"

#include "OgreSceneManager.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/SceneNodeUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/MovableObjectUserData.h"
#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/RayUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/Event/SystemEventListenerObjects.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/Particles/ParticleSystemUserData.h"

#include "OgreItem.h"
#include "OgreLight.h"
#include "OgreParticleSystem.h"
#include "OgreCamera.h"

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

        Ogre::uint32 targetMask = Ogre::SceneManager::QUERY_ENTITY_DEFAULT_MASK;
        if(sq_gettop(vm) > 2){
            SQInteger val;
            ASSERT_SQ_RESULT(sq_getinteger(vm, 3, &val));
            targetMask = static_cast<Ogre::uint32>(val);
        }

        //Optimisation This could be shared so it's not re-created and destroyed each call.
        Ogre::RaySceneQuery* sceneQuery = _scene->createRayQuery(targetRay, targetMask);
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

    SQInteger SceneNamespace::createParticleSystem(HSQUIRRELVM vm){
        const SQChar *particle;
        sq_getstring(vm, -1, &particle);

        Ogre::ParticleSystem* ps = _scene->createParticleSystem(particle);
        //Particles default to render queue 110.
        //ParticleSystemUserData::createUserDataFromPointer(vm, ps);
        MovableObjectUserData::movableObjectToUserData(vm, (Ogre::MovableObject*)ps, MovableObjectType::ParticleSystem);

        return 1;
    }

    SQInteger SceneNamespace::createCamera(HSQUIRRELVM vm){
        const SQChar *name;
        sq_getstring(vm, -1, &name);

        Ogre::Camera* camera = 0;
        try{
            camera = _scene->createCamera(name);
        }catch(Ogre::ItemIdentityException e){
            return sq_throwerror(vm, "Camera with that name already exists.");
        }
        //Ogre attaches the newly created camera to a scene node by default, so detach it.
        camera->detachFromParent();

        MovableObjectUserData::movableObjectToUserData(vm, (Ogre::MovableObject*)camera, MovableObjectType::Camera);

        return 1;
    }

    SQInteger SceneNamespace::registerChunkCallback(HSQUIRRELVM vm){
        SQObject closure;
        sq_getstackobj(vm, -1, &closure);
        SystemEventListenerObjects::registerListenerForType(SystemEventListenerObjects::CHUNK, closure);

        return 0;
    }

    SQInteger SceneNamespace::getNumDataPoints(HSQUIRRELVM vm){
        const RecipeData* data = SystemEventListenerObjects::mCurrentRecipeData;
        if(!data) return sq_throwerror(vm, "Not processing a chunk callback.");
        if(!data->dataPoints){
            //If the vector does not exist because there were no data points.
            sq_pushinteger(vm, 0);
            return 1;
        }
        sq_pushinteger(vm, data->dataPoints->size());

        return 1;
    }

    SQInteger SceneNamespace::getDataPointAt(HSQUIRRELVM vm){
        SQInteger idx = 0;
        sq_getinteger(vm, -2, &idx);
        const RecipeData* data = SystemEventListenerObjects::mCurrentRecipeData;
        if(!data) return sq_throwerror(vm, "Not processing a chunk callback.");
        if(
            !data->dataPoints || idx >= data->dataPoints->size()
        ) return sq_throwerror(vm, "Invalid index.");

        if(sq_getsize(vm, -2) > 4) return sq_throwerror(vm, "The provided array was too small.");

        const DataPointEntry& e = (*data->dataPoints)[idx];
        sq_pushinteger(vm, 0);
        SlotPositionClass::createNewInstance(vm, SlotPosition(data->coord.chunkX(), data->coord.chunkY(), e.pos));
        sq_rawset(vm, 3);
        sq_pushinteger(vm, 1);
        sq_pushinteger(vm, e.type);
        sq_rawset(vm, 3);
        sq_pushinteger(vm, 2);
        sq_pushinteger(vm, e.subType);
        sq_rawset(vm, 3);
        sq_pushinteger(vm, 3);
        sq_pushinteger(vm, e.userData);
        sq_rawset(vm, 3);


        return 0;
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

        /**SQFunction
        @name createCamera
        @desc Create a camera, which can be attached to a scene node to perform rendering.
        @param1:String:The name of the camera. This must be unique.
        @returns A camera object.
        */
        ScriptUtils::addFunction(vm, createCamera, "createCamera", 2, ".s");

        ScriptUtils::addFunction(vm, createLight, "createLight");
        /**SQFunction
        @name testRayForSlot
        @desc Perform a ray test on the Ogre scene, finding a slot position of the nearest collision.
        @param1:Ray: A ray object to test with.
        @returns A SlotPosition if a collision was found. Null if nothing was found.
        */
        ScriptUtils::addFunction(vm, testRayForSlot, "testRayForSlot", -2, ".ui");

        ScriptUtils::addFunction(vm, createParticleSystem, "createParticleSystem", 2, ".s");

        //Chunk callback related stuff.

        /**SQFunction
        @name registerChunkCallback
        @desc Register a callback to be called during chunk events.
        @param1:Closure: The closure which should be called.
        */
        ScriptUtils::addFunction(vm, registerChunkCallback, "registerChunkCallback", 2, ".c");
        /**SQFunction
        @name getNumDataPoints
        @desc Get the number of data points in this chunk. This function will throw an error if not called during chunk construction.
        */
        ScriptUtils::addFunction(vm, getNumDataPoints, "getNumDataPoints");
        /**SQFunction
        @name getDataPointAt
        @desc Fill the provided array with the data point data at index.
        The provided array must be a size of at least 4. If not an error will be thrown.
        This function will throw an error if not called during chunk construction.
        The format of the array will be, position, type, subtype, userdata.
        */
        ScriptUtils::addFunction(vm, getDataPointAt, "getDataPointAt", 3, ".ia");
    }

}
