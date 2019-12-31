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

    SQInteger SettingsNamespace::getSaveDirectoryViable(HSQUIRRELVM vm){
        sq_pushbool(vm, SystemSettings::isSaveDirectoryViable());

        return 1;
    }

    void SettingsNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, getDataDirectory, "getDataDirectory");
        ScriptUtils::addFunction(vm, getMasterDirectory, "getMasterDirectory");
        ScriptUtils::addFunction(vm, getWorldSlotSize, "getWorldSlotSize");
        ScriptUtils::addFunction(vm, getCurrentRenderSystem, "getCurrentRenderSystem");

        ScriptUtils::addFunction(vm, getSaveDirectoryViable, "getSaveDirectoryViable");
    }

    void SettingsNamespace::setupConstants(HSQUIRRELVM vm){
        sq_pushroottable(vm);

        ScriptUtils::declareConstant(vm, "_RenderSystemUnset", (int)SystemSettings::RenderSystemTypes::RENDER_SYSTEM_UNSET);
        ScriptUtils::declareConstant(vm, "_RenderSystemD3D11", (int)SystemSettings::RenderSystemTypes::RENDER_SYSTEM_D3D11);
        ScriptUtils::declareConstant(vm, "_RenderSystemMetal", (int)SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL);
        ScriptUtils::declareConstant(vm, "_RenderSystemOpenGL", (int)SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL);

        sq_pop(vm, 1);
    }

}
