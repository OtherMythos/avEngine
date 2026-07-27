#include "PluginNamespace.h"

#include "System/SystemSetup/SystemSettings.h"

namespace AV{

    SQInteger PluginNamespace::getPlugins(HSQUIRRELVM vm){
        const std::vector<SystemSettings::PluginEntry>& entries = SystemSettings::getPluginEntries();

        sq_newarray(vm, 0);
        for(const SystemSettings::PluginEntry& e : entries){
            sq_newtableex(vm, 4);

            sq_pushstring(vm, _SC("name"), 4);
            sq_pushstring(vm, e.name.c_str(), e.name.size());
            sq_newslot(vm, -3, SQFalse);

            sq_pushstring(vm, _SC("description"), 11);
            sq_pushstring(vm, e.description.c_str(), e.description.size());
            sq_newslot(vm, -3, SQFalse);

            sq_pushstring(vm, _SC("version"), 7);
            sq_pushstring(vm, e.version.c_str(), e.version.size());
            sq_newslot(vm, -3, SQFalse);

            sq_pushstring(vm, _SC("directory"), 9);
            sq_pushstring(vm, e.directory.c_str(), e.directory.size());
            sq_newslot(vm, -3, SQFalse);

            sq_arrayappend(vm, -2);
        }

        return 1;
    }

    SQInteger PluginNamespace::isLoaded(HSQUIRRELVM vm){
        const SQChar *pluginName;
        sq_getstring(vm, 2, &pluginName);

        bool found = false;
        for(const SystemSettings::PluginEntry& e : SystemSettings::getPluginEntries()){
            if(e.name != pluginName) continue;
            found = true;
            break;
        }

        sq_pushbool(vm, found);

        return 1;
    }

    /**SQNamespace
    @name _plugin
    @desc A namespace to query the plugins the engine was asked to load.
    */
    void PluginNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name getPlugins
        @desc List the plugins which were found and loaded.
        @returns An array of tables, each containing name, description, version and directory entries.
        */
        ScriptUtils::addFunction(vm, getPlugins, "getPlugins");
        /**SQFunction
        @name isLoaded
        @param1:pluginName: The name a plugin declares in its avPlugin.cfg file.
        @returns True if a plugin of that name was loaded.
        */
        ScriptUtils::addFunction(vm, isLoaded, "isLoaded", 2, ".s");
    }
}
