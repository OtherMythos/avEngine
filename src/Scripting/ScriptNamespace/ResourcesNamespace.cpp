#include "ResourcesNamespace.h"

#include "System/Util/PathUtils.h"
#include "System/EngineFlags.h"

#include "Logger/Log.h"

#include <OgreResourceGroupManager.h>

namespace AV{

    SQInteger ResourcesNamespace::addResourceLocation(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, 2, &path);

        std::string outString;
        formatResToPath(path, outString);

        const SQChar *locType;
        sq_getstring(vm, 3, &locType);

        const SQChar *resGroupName = "General";
        if(sq_gettop(vm) >= 4){
            sq_getstring(vm, 4, &resGroupName);
        }

        AV_INFO("Added resource location {} of type '{}' to group '{}'", outString, locType, resGroupName);
        Ogre::ResourceGroupManager::getSingleton()
            .addResourceLocation(outString, locType, resGroupName);

        return 0;
    }

    SQInteger ResourcesNamespace::removeResourceLocation(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, 2, &path);

        std::string outString;
        formatResToPath(path, outString);

        if(!EngineFlags::resourceGroupValid(outString)){
            return sq_throwerror(vm, "Attempting to remove internal resource group.");
        }

        AV_INFO("Removing resource location {}", outString);
        Ogre::ResourceGroupManager::getSingleton()
            .removeResourceLocation(outString);

        return 0;
    }

    SQInteger ResourcesNamespace::initialiseAllResourceGroups(HSQUIRRELVM vm){
        Ogre::ResourceGroupManager::getSingleton()
            .initialiseAllResourceGroups(false);

        return 0;
    }

    SQInteger ResourcesNamespace::initialiseResourceGroup(HSQUIRRELVM vm){
        const SQChar *resGroup;
        sq_getstring(vm, 2, &resGroup);

        try{
            Ogre::ResourceGroupManager::getSingleton()
                .initialiseResourceGroup(resGroup, false);
        }catch(Ogre::Exception e){
            return sq_throwerror(vm, e.getDescription().c_str());
        }

        return 0;
    }

    SQInteger ResourcesNamespace::prepareResourceGroup(HSQUIRRELVM vm){
        const SQChar *resGroup;
        sq_getstring(vm, 2, &resGroup);

        try{
            Ogre::ResourceGroupManager::getSingleton().prepareResourceGroup(resGroup, true);
        }catch(Ogre::Exception e){
            return sq_throwerror(vm, e.getDescription().c_str());
        }

        return 0;
    }

    SQInteger ResourcesNamespace::destroyResourceGroup(HSQUIRRELVM vm){
        const SQChar *resGroup;
        sq_getstring(vm, 2, &resGroup);

        if(!EngineFlags::resourceGroupValid(resGroup)){
            return sq_throwerror(vm, "Attempted to remove internal resource group.");
        }

        try{
            Ogre::ResourceGroupManager::getSingleton().destroyResourceGroup(resGroup);
        }catch(Ogre::Exception e){
            return sq_throwerror(vm, e.getDescription().c_str());
        }

        return 0;
    }

    SQInteger ResourcesNamespace::getResourceGroups(HSQUIRRELVM vm){

        Ogre::StringVector groupsVec = Ogre::ResourceGroupManager::getSingleton()
            .getResourceGroups();

        sq_newarray(vm, 0);
        for(int i = 0; i < groupsVec.size(); i++){
            const std::string& str = groupsVec[i];
            sq_pushstring(vm, str.c_str(), str.size());
            sq_arrayappend(vm, -2);
        }

        return 1;
    }

    /**SQNamespace
    @name _resources
    @desc A namespace to effect resource locations and the Ogre resource system.
    */
    void ResourcesNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name addResourceLocation
        @desc Add a resource location.
        @param1:ResPath:The path of the resource group.
        @param2:String:Location type.
        @param3:String:Resource group name. Defaults to 'General'
        */
        ScriptUtils::addFunction(vm, addResourceLocation, "addResourceLocation", -3, ".sss");
        /**SQFunction
        @name removeResourceLocation
        @desc Remove a resource location.
        @param1:String:location name.
        */
        ScriptUtils::addFunction(vm, removeResourceLocation, "removeResourceLocation", 2, ".s");
        /**SQFunction
        @name initialiseAllResourceGroups
        @desc Initialise all resource groups which have yet to be initialised.
        */
        ScriptUtils::addFunction(vm, initialiseAllResourceGroups, "initialiseAllResourceGroups");
        /**SQFunction
        @name initialiseResourceGroup
        @desc Initialise a resource group by name.
        @param1:String:The name of the resource group.
        */
        ScriptUtils::addFunction(vm, initialiseResourceGroup, "initialiseResourceGroup", 2, ".s");
        /**SQFunction
        @name prepareResourceGroup
        @desc Prepares any resources which are part of a named group.
        @param1:String:The name of the resource group.
        */
        ScriptUtils::addFunction(vm, prepareResourceGroup, "prepareResourceGroup", 2, ".s");
        /**SQFunction
        @name destroyResourceGroup
        @desc Destroys a resource group, clearing it first, destroying the resources which are part of it, and then removing it from the list of resource groups.
        @param1:String:The name of the resource group.
        */
        ScriptUtils::addFunction(vm, destroyResourceGroup, "destroyResourceGroup", 2, ".s");
        /**SQFunction
        @name getResourceGroups
        @desc Get a list of all currently defined resource groups.
        @returns An array containing strings  of all resource groups.
        */
        ScriptUtils::addFunction(vm, getResourceGroups, "getResourceGroups");
    }
}
