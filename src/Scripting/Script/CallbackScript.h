#pragma once

#include "BaseScript.h"
#include <map>
#include <vector>

namespace AV{
    /**
     A class to encapsulate callback functionality of scripts.
     Callback scripts are intended to simply contain a list of functions which can be executed individually by this class.
     */
    class CallbackScript : public BaseScript{
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
         @return
         Whether or not this call was successful.
         */
        bool call(const Ogre::String& functionName);
        
        bool call(int closureId);
        
        /**
         Get the int id of a callback.
         This id can later be used for more efficient calling.
         
         @return
         A positive number if a callback with that name was found. -1 if not.
         */
        int getCallbackId(const Ogre::String& functionName);
        
        /**
         Whether or not a callback with the specified name exists in this callback script.
         
         @param functionName
         The name of the callback that should be executed.
         */
        bool containsCallback(const Ogre::String& functionName);

        /**
         Release this script and all the resources held by it.
         */
        void release();

    private:
        HSQUIRRELVM mVm;
        HSQOBJECT mMainClosure;
        HSQOBJECT mMainTable;

        bool _compileMainClosure(const Ogre::String& path);
        bool _createMainTable();
        bool _callMainClosure();
        bool _parseClosureTable();
        
        bool _call(int closureId);

        bool mPrepared = false;
        bool mInitialised = false;

        std::vector<HSQOBJECT> mClosures;
        std::map<Ogre::String, int> mClosureMap;
    };
}
