#include "ResourcesNamespace.h"

#include "System/Util/PathUtils.h"
#include "System/EngineFlags.h"

#include "System/OgreSetup/SetupHelpers.h"
#include "Logger/Log.h"

#include "OgreResourceGroupManager.h"
#include "OgreParticleSystemManager.h"
#include "OgreParticleSystem.h"

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

    SQInteger ResourcesNamespace::resourceModifiedTime(HSQUIRRELVM vm){
        const SQChar *group;
        sq_getstring(vm, 2, &group);

        const SQChar *filename;
        sq_getstring(vm, 3, &filename);

        time_t time = 0;
        try{
            time = Ogre::ResourceGroupManager::getSingleton()
                .resourceModifiedTime(group, filename);
        }catch(Ogre::Exception e){
            return sq_throwerror(vm, e.getDescription().c_str());
        }

        sq_pushinteger(vm, static_cast<SQInteger>(time));

        return 1;
    }

    SQInteger ResourcesNamespace::parseOgreResourcesFile(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, 2, &path);

        std::string outString;
        formatResToPath(path, outString);

        SetupHelpers::parseOgreResourcesFile(outString);

        return 0;
    }

    SQInteger ResourcesNamespace::getScriptForParticleSystem(HSQUIRRELVM vm){
        const SQChar *path;
        sq_getstring(vm, 2, &path);

        Ogre::ParticleSystem* system = Ogre::ParticleSystemManager::getSingleton().getTemplate(path);
        if(!system) return sq_throwerror(vm, (std::string("No script found for particle system ") + path).c_str());
        const Ogre::String& origin = system->getOrigin();

        sq_pushstring(vm, origin.c_str(), origin.size());

        return 1;
    }

    SQInteger ResourcesNamespace::findGroupContainingResource(HSQUIRRELVM vm){
        const SQChar *resName;
        sq_getstring(vm, 2, &resName);

        try{
            const Ogre::String& groupName = Ogre::ResourceGroupManager::getSingleton()
                .findGroupContainingResource(resName);
            sq_pushstring(vm, groupName.c_str(), groupName.size());
        }catch(Ogre::Exception e){
            return sq_throwerror(vm, e.getDescription().c_str());
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
        /**SQFunction
        @name resourceModifiedTime
        @desc Get the time as an integer since the given resource was modified.
        @returns An integer representing time.
        */
        ScriptUtils::addFunction(vm, resourceModifiedTime, "resourceModifiedTime", 3, ".ss");
        /**SQFunction
        @name parseOgreResourcesFile
        @desc Parse an Ogre resources file, adding its resource locations to the system.
        @param1:ResPath:The path to the resources file to parse.
        */
        ScriptUtils::addFunction(vm, parseOgreResourcesFile, "parseOgreResourcesFile", 2, ".s");
        /**SQFunction
        @name getScriptForParticleSystem
        @desc Query a particle system template by name and return the script where it was defined.
        @param1:String:Name of the particle sytem to query.
        @returns The name of the script which contained the template.
        */
        ScriptUtils::addFunction(vm, getScriptForParticleSystem, "getScriptForParticleSystem", 2, ".s");
        /**SQFunction
        @name findGroupContainingResource
        @desc Find the group in which a resource exists. Throws an error if the group could not be determined.
        @param1:String:Name of the resource script to check.
        @returns The name of the resource group the script is defined in.
        */
        ScriptUtils::addFunction(vm, findGroupContainingResource, "findGroupContainingResource", 2, ".s");
    }
}
