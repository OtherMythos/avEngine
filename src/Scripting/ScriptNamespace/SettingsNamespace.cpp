#include "SettingsNamespace.h"

#include "System/SystemSetup/SystemSettings.h"

namespace AV{
    SQInteger SettingsNamespace::getDataDirectory(HSQUIRRELVM vm){
        const std::string& dataPath = SystemSettings::getDataPath();
        sq_pushstring(vm, _SC(dataPath.c_str()), -1);
        
        return 1;
    }
    
    SQInteger SettingsNamespace::getMasterDirectory(HSQUIRRELVM vm){
        const std::string& masterPath = SystemSettings::getMasterPath();
        sq_pushstring(vm, _SC(masterPath.c_str()), -1);
        
        return 1;
    }
    
    SQInteger SettingsNamespace::getWorldSlotSize(HSQUIRRELVM vm){
        sq_pushinteger(vm, SystemSettings::getWorldSlotSize());
        
        return 1;
    }
    
    SQInteger SettingsNamespace::getCurrentRenderSystem(HSQUIRRELVM vm){
        sq_pushinteger(vm, (int)SystemSettings::getCurrentRenderSystem());
        
        return 1;
    }
    
    void SettingsNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, getDataDirectory, "getDataDirectory");
        _addFunction(vm, getMasterDirectory, "getMasterDirectory");
        _addFunction(vm, getWorldSlotSize, "getWorldSlotSize");
        _addFunction(vm, getCurrentRenderSystem, "getCurrentRenderSystem");
    }
    
    void SettingsNamespace::setupConstants(HSQUIRRELVM vm){
        sq_pushroottable(vm);
        
        _declareConstant(vm, "_RenderSystemUnset", (int)SystemSettings::RenderSystemTypes::RENDER_SYSTEM_UNSET);
        _declareConstant(vm, "_RenderSystemD3D11", (int)SystemSettings::RenderSystemTypes::RENDER_SYSTEM_D3D11);
        _declareConstant(vm, "_RenderSystemMetal", (int)SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL);
        _declareConstant(vm, "_RenderSystemOpenGL", (int)SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL);
        
        sq_pop(vm, 1);
    }

}
