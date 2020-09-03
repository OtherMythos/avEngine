#pragma once

#include "System/EnginePrerequisites.h"

#include <string>
#include <map>
#include <vector>

#include "OgreIdString.h"

namespace AV{
    class CallbackScript;
    class ScriptEventManager;

    typedef std::shared_ptr<CallbackScript> CallbackScriptPtr;

    /**
    A class to manage the creation and lifetime of callback scripts.
    These scripts are distributed as a shared pointer, meaning that if lots of components use the same script, it will only be stored once.
    */
    class ScriptManager{
    public:
        typedef unsigned int ScriptId;
        static const ScriptId INVALID_SCRIPT_ID = 0xFFFFFF;

    public:
        ScriptManager();
        ~ScriptManager();

        /**
        Load a res path representing the path to the target script.

        @returns
        A shared pointer to the callback script. When completely dereferenced the script will be destroyed.
        This function has the potential to return a blank pointer in the case of an error during script loading, for instance if there is no file at the script path.
        */
        CallbackScriptPtr loadScript(const std::string& scriptPath);

    private:
        typedef std::weak_ptr<CallbackScript> WeakScriptPtr;

        typedef std::pair<std::string, ScriptId> ScriptPathEntry;
        //Reference and script.
        //typedef std::pair<uint16, WeakScriptPtr> CallbackScriptEntry;

        std::shared_ptr<ScriptEventManager> mScriptEventManager;

        //A list of string paths for each script.
        std::map<Ogre::IdString, ScriptPathEntry> mScriptPaths;
        std::vector<WeakScriptPtr> mCallbackScripts;

        ScriptId _findScript(const Ogre::IdString& id);
        ScriptId _createLoadedSlot(const Ogre::IdString& hashedPath, const std::string &scriptPath, WeakScriptPtr scripts);
        ScriptId _getAvailableIndex();

        /**
        Remove a callback script from the internal data.
        This function should only be called as part of the shared pointer destruction.
        */
        void _removeScript(ScriptId id);

        //void _removeScript(ScriptId id);

        //Called on shared pointer destruction.
        static void _destroyCallbackScript(CallbackScript* script);
    };
}
