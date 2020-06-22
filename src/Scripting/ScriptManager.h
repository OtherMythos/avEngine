#pragma once

#include "System/EnginePrerequisites.h"

#include <string>
#include <map>
#include <vector>

#include "OgreIdString.h"

namespace AV{
    class CallbackScript;

    typedef std::shared_ptr<CallbackScript> CallbackScriptPtr;

    class ScriptManager{
    public:
        typedef unsigned int ScriptId;
        static const ScriptId INVALID_SCRIPT_ID = 0xFFFFFF;

    public:
        ScriptManager();
        ~ScriptManager();

        /**
        Load a res path representing the path to the target script.
        */
        CallbackScriptPtr loadScript(const std::string& scriptPath);

    private:
        typedef std::weak_ptr<CallbackScript> WeakScriptPtr;

        typedef std::pair<std::string, ScriptId> ScriptPathEntry;
        //Reference and script.
        //typedef std::pair<uint16, WeakScriptPtr> CallbackScriptEntry;

        //A list of string paths for each script.
        std::map<Ogre::IdString, ScriptPathEntry> mScriptPaths;
        std::vector<WeakScriptPtr> mCallbackScripts;

        ScriptId _findScript(const Ogre::IdString& id);
        ScriptId _createLoadedSlot(const Ogre::IdString& hashedPath, const std::string &scriptPath, WeakScriptPtr scripts);
        ScriptId _getAvailableIndex();

        //void _removeScript(ScriptId id);

        //Called on shared pointer destruction.
        static void _destroyCallbackScript(CallbackScript* script);
    };
}
