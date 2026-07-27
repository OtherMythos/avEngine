#pragma once

#include <string>
#include <vector>
#include <memory>
#include <squirrel.h>
#include "System/EnginePrerequisites.h"

namespace AV{
    class CallbackScript;

    /**
    Runs the Squirrel half of the engine's plugins.

    A plugin which declares an EntryFile in its avPlugin.cfg has that file loaded as a callback
    script, and any of the functions below that it declares are called at the appropriate time.
    All of them are optional.

        start()                 once, after the native plugins have registered themselves
        update() / update(dt)   each fixed update
        sceneSafeUpdate()       once per rendered frame, while the scene is guaranteed clean
        end()                   once, before the script vm is shut down

    Plugins are started in the order they appear in the avSetup.cfg Plugins list. There is no
    dependency resolution between plugins.
    */
    class ScriptPluginManager{
    public:
        ScriptPluginManager();
        ~ScriptPluginManager();

        /**
        Load the entry file of each script plugin and call its start function.
        This should happen after the native plugins have been loaded, so that a plugin's script
        can rely on any namespace its own native half registered.
        */
        void initialise();

        /**
        Call the end function of each plugin.
        Must be called while the script vm is still open.
        */
        void shutdown();

        void update();

        /**
        Update the plugins at a point where the scene is guaranteed to be clean.
        Call this before any other functions of the main update loop.
        */
        void updateSceneSafe();

        /**
        Set the fixed delta time to pass to plugin update functions.
        Should be called before update() each fixed step.
        */
        void setFixedDeltaTime(float dt) { mFixedDeltaTime = dt; }

    private:
        struct PluginScriptEntry{
            std::shared_ptr<CallbackScript> s;
            std::string name;
            int startId, updateId, endId, sceneSafeUpdateId;
            //Number of parameters for the update closure (including implicit 'this').
            uint8 updateParamCount;
        };

        float mFixedDeltaTime = 1.0f / 60.0f;

        //Static storage so the PopulateFunction callback can access it.
        static float sCurrentDeltaTime;
        static SQInteger _pushDeltaTime(HSQUIRRELVM vm);

        std::vector<PluginScriptEntry> mPlugins;
    };
}
