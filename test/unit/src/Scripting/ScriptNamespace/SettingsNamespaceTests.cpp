#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "../ScriptTestHelper.h"
#include "System/SystemSetup/SystemSettings.h"

TEST(SettingsNamespaceTests, getDataDirectory){
    AV::SystemSettings::_dataPath = "A path";
    
    bool output = false;
    ASSERT_TRUE(ScriptTestHelper::executeStringBool("return _settings.getDataDirectory() == \"A path\"", &output));
    
    ASSERT_TRUE(output);
}

TEST(SettingsNamespaceTests, getMasterDirectory){
    AV::SystemSettings::_masterPath = "A path";
    
    bool output = false;
    ASSERT_TRUE(ScriptTestHelper::executeStringBool("return _settings.getMasterDirectory() == \"A path\"", &output));
    
    ASSERT_TRUE(output);
}

TEST(SettingsNamespaceTests, getWorldSlotSizeTest){
    AV::SystemSettings::_worldSlotSize = 14;
    
    int output = 0;
    ASSERT_TRUE(ScriptTestHelper::executeStringInt("return _settings.getWorldSlotSize();", &output));
    
    ASSERT_EQ(output, AV::SystemSettings::_worldSlotSize);
}

TEST(SettingsNamespaceTests, getCurrentRenderSystemTest){
    typedef std::pair<AV::SystemSettings::RenderSystemTypes, std::string> pairEntry;
    
    std::vector<pairEntry> pairs = {
        {AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_UNSET, "_RenderSystemUnset"},
        {AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_D3D11, "_RenderSystemD3D11"},
        {AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL, "_RenderSystemMetal"},
        {AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL, "_RenderSystemOpenGL"},
    };
    
    for(const pairEntry& p : pairs){
        AV::SystemSettings::mCurrentRenderSystem = p.first;
        
        bool output = false;
        ASSERT_TRUE(ScriptTestHelper::executeStringBool("return _settings.getCurrentRenderSystem() == " + p.second, &output));
        
        ASSERT_TRUE(output);
    }
}
