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

    SQInteger SettingsNamespace::getEngineFeatures(HSQUIRRELVM vm){
        SQInteger features = 0;

        #ifdef TEST_MODE
            features |= FEATURE_TEST_MODE;
        #endif
        #ifdef DEBUGGING_TOOLS
            features |= FEATURE_DEBUGGING_TOOLS;
        #endif

        sq_pushinteger(vm, features);

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

    /**SQNamespace
    @name _settings
    @desc A utility namespace to generate random numbers.
    */
    void SettingsNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name getDataDirectory
        @returns A string path to the data directory.
        */
        ScriptUtils::addFunction(vm, getDataDirectory, "getDataDirectory");
        /**SQFunction
        @name getMasterDirectory
        @returns A string path to the master directory.
        */
        ScriptUtils::addFunction(vm, getMasterDirectory, "getMasterDirectory");
        /**SQFunction
        @name getWorldSlotSize
        @returns An integer representing the slot size.
        */
        ScriptUtils::addFunction(vm, getWorldSlotSize, "getWorldSlotSize");
        /**SQFunction
        @name getCurrentRenderSystem
        @returns An integer representing the current render system.
        */
        ScriptUtils::addFunction(vm, getCurrentRenderSystem, "getCurrentRenderSystem");

        /**SQFunction
        @name getSaveDirectoryViable
        @returns A boolean representing whether or not the save directory is viable.
        */
        ScriptUtils::addFunction(vm, getSaveDirectoryViable, "getSaveDirectoryViable");

        /**SQFunction
        @name getEngineFeatures
        @returns A bitmask integer populated with engine features.
        */
        ScriptUtils::addFunction(vm, getEngineFeatures, "getEngineFeatures");

        /**SQFunction
        @name getUserSetting
        @desc Retreive a user setting. These settings can be specified by the user in the avSetup.cfg file.
        @param1:settingName:The name of the setting to retreive.
        @returns Either a float, int, string or boolean. If a user setting was not found for that name null will be returned.
        */
        ScriptUtils::addFunction(vm, getUserSetting, "getUserSetting", 2, ".s");
    }

    void SettingsNamespace::setupConstants(HSQUIRRELVM vm){
        sq_pushroottable(vm);

        ScriptUtils::declareConstant(vm, "_RenderSystemUnset", (int)SystemSettings::RenderSystemTypes::RENDER_SYSTEM_UNSET);
        ScriptUtils::declareConstant(vm, "_RenderSystemD3D11", (int)SystemSettings::RenderSystemTypes::RENDER_SYSTEM_D3D11);
        ScriptUtils::declareConstant(vm, "_RenderSystemMetal", (int)SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL);
        ScriptUtils::declareConstant(vm, "_RenderSystemOpenGL", (int)SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL);

        ScriptUtils::declareConstant(vm, "_FeatureDebuggingTools", FEATURE_DEBUGGING_TOOLS);
        ScriptUtils::declareConstant(vm, "_FeatureTestMode", FEATURE_TEST_MODE);

        sq_pop(vm, 1);
    }

}
