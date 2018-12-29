#include "ScriptManager.h"
#include "Logger/Log.h"
#include "scriptNamespace/CameraNamespace.h"
#include "scriptNamespace/MeshNamespace.h"

#include <iostream>

#include <sqstdio.h>
#include <sqstdmath.h>

#ifdef SQUNICODE
#define scvprintf vwprintf
#else
#define scvprintf vprintf
#endif

namespace AV {
    HSQUIRRELVM ScriptManager::_sqvm = sq_open(1024);

    void printfunc(HSQUIRRELVM v, const SQChar *s, ...){
        va_list arglist;
        va_start(arglist, s);
        scvprintf(s, arglist);
        va_end(arglist);
        std::cout << '\n';
    }

    void ScriptManager::initialise(){
        _setupVM(_sqvm);
    }

    void ScriptManager::runScript(const std::string &scriptPath){
        AV_INFO("Running Script {}", scriptPath);
        sq_pushroottable(_sqvm);
        if(SQ_SUCCEEDED(sqstd_dofile(_sqvm, _SC(scriptPath.c_str()), 0, 1))){
            AV_INFO("Succeeded");
        }else{
            AV_ERROR("There was a problem loading that script file.");
        }
    }

    void ScriptManager::injectPointers(Ogre::Camera *camera, Ogre::SceneManager* sceneManager){
        CameraNamespace::_camera = camera;
        MeshNamespace::_sceneManager = sceneManager;
    }

    void ScriptManager::_setupVM(HSQUIRRELVM vm){
        sq_setprintfunc(vm, printfunc, NULL);

        sq_pushroottable(vm);

        sqstd_register_mathlib(vm);

        CameraNamespace cameraNamespace;
        MeshNamespace meshNamespace;
        _createCameraNamespace(vm, cameraNamespace);
        _createMeshNamespace(vm, meshNamespace);

        sq_pop(vm,1);
    }

    void ScriptManager::_createCameraNamespace(HSQUIRRELVM vm, CameraNamespace &cameraNamespace){
        sq_pushstring(vm, _SC("_camera"), -1);
        sq_newtable(vm);

        cameraNamespace.setupNamespace(vm);

        sq_newslot(vm, -3 , false);

    }
    
    void ScriptManager::_createMeshNamespace(HSQUIRRELVM vm, MeshNamespace &meshNamespace){
        sq_pushstring(vm, _SC("_mesh"), -1);
        sq_newtable(vm);
        
        meshNamespace.setupNamespace(vm);
        
        sq_newslot(vm, -3 , false);
        
    }
}
