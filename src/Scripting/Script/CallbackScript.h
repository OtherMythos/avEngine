#pragma once

#include "BaseScript.h"
#include <map>
#include <vector>
#include "OgreIdString.h"
#include "System/EnginePrerequisites.h"

namespace AV{
    class EntityCallbackScript;
    class ScriptManager;

    typedef SQInteger(*PopulateFunction)(HSQUIRRELVM vm);

    /**
     A class to encapsulate callback functionality of scripts.
     Callback scripts are intended to simply contain a list of functions which can be executed individually by this class.
     It allows precise control over how these functions are called, and what parameters are passed to them.

     Generally callback scripts should be created using the ScriptManager class, as this allows automatic managing on script instance lifetime.
     */
    class CallbackScript : public BaseScript{
        friend EntityCallbackScript;

    public:
        CallbackScript(HSQUIRRELVM vm);
        CallbackScript();
        ~CallbackScript();

        /**
         Setup the script by specifying a path to a squirrel file.
         This function is responsible for parsing the contents of the file, and preparing the closures for execution.
         The script must first be initalised before this can be called.

         @param path
         The path to the script that should be processed.
         @return
         Whether or not the preparation work was successful.
         */
        bool prepare(const Ogre::String& path);
        /**
         Initialise this script with a vm. Either this or the vm constructor needs to be called before the script can be used.
         */
        void initialise(HSQUIRRELVM vm);

        /**
         Call a callback function.
         This function expects the script to have been prepared and initialised before this will work.

         @param functionName
         The name of the callback that should be executed.
         @param func
         A stack populating function.
         During part of the call procedure the squirrel stack needs to be prepared with the appropriate parameter variables.
         However these variables are often very specific.
         So this function pointer is used to populate the stack however the user needs, meaning any sort of variables can be passed in.
         If the called function takes no parameters then this can just be left as a null pointer.
         @return
         Whether or not this call was successful.
         */
        bool call(const Ogre::String& functionName, PopulateFunction func = 0);

        bool call(int closureId, PopulateFunction func = 0);

        /**
         Get the int id of a callback.
         This id can later be used for more efficient calling.

         @return
         A positive number if a callback with that name was found. -1 if not.
         */
        int getCallbackId(const Ogre::String& functionName);

        /**
        Retrieve the number of parameters associated with a closure.
        This value includes the invisible 'this' parameter.
        */
        uint8 getParamsForCallback(int closureId) const;

        /**
         Release this script and all the resources held by it.
         */
        void release();

        ScriptManager* mCreatorClass = 0;
        unsigned int mScriptId = 0;

    private:
        HSQUIRRELVM mVm;
        HSQOBJECT mMainClosure;
        HSQOBJECT mMainTable;

        bool _compileMainClosure(const Ogre::String& path);
        bool _createMainTable();
        bool _callMainClosure();
        bool _parseClosureTable();

        bool _call(int closureId, PopulateFunction func);

        bool mPrepared = false;
        bool mInitialised = false;

        //Closure and the number of parameters it contains.
        typedef std::pair<HSQOBJECT, uint8> ClosureEntry;
        std::vector<ClosureEntry> mClosures;
        std::map<Ogre::IdString, int> mClosureMap;
    };
}
