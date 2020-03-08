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

    SQInteger SettingsNamespace::getUserSetting(HSQUIRRELVM vm){
        const SQChar *settingName;
        sq_getstring(vm, -1, &settingName);

        void* target = 0;
        SystemSettings::UserSettingType type;
        bool found = SystemSettings::getUserSetting(settingName, &target, &type);
        if(!found){
            return 0; //Return null
        }

        assert(target);
        switch(type){
            case SystemSettings::UserSettingType::String:{
                std::string* str = reinterpret_cast<std::string*>(target);
                sq_pushstring(vm, str->c_str(), -1);
                break;
            }
            case SystemSettings::UserSettingType::Int:{
                int* i = reinterpret_cast<int*>(target);
                sq_pushinteger(vm, *i);
                break;
            }
            case SystemSettings::UserSettingType::Float:{
                float* f = reinterpret_cast<float*>(target);
                sq_pushfloat(vm, *f);
                break;
            }
            case SystemSettings::UserSettingType::Bool:{
                bool* b = reinterpret_cast<bool*>(target);
                sq_pushbool(vm, *b);
                break;
            }
            default: assert(false);
        }

        return 1;
    }

    void SettingsNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, getDataDirectory, "getDataDirectory");
        ScriptUtils::addFunction(vm, getMasterDirectory, "getMasterDirectory");
        ScriptUtils::addFunction(vm, getWorldSlotSize, "getWorldSlotSize");
        ScriptUtils::addFunction(vm, getCurrentRenderSystem, "getCurrentRenderSystem");

        ScriptUtils::addFunction(vm, getSaveDirectoryViable, "getSaveDirectoryViable");

        ScriptUtils::addFunction(vm, getUserSetting, "getUserSetting", 2, ".s");
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
