#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "../ScriptTestHelper.h"
#include "unit/src/TestAccessors.h"

TEST(SettingsNamespaceTests, getDataDirectory){
    TestableSystemSettings::_dataPath = "A path";

    bool output = false;
    ASSERT_TRUE(ScriptTestHelper::executeStringBool("return _settings.getDataDirectory() == \"A path\"", &output));

    ASSERT_TRUE(output);
}

TEST(SettingsNamespaceTests, getMasterDirectory){
    TestableSystemSettings::_masterPath = "A path";

    bool output = false;
    ASSERT_TRUE(ScriptTestHelper::executeStringBool("return _settings.getMasterDirectory() == \"A path\"", &output));

    ASSERT_TRUE(output);
}

TEST(SettingsNamespaceTests, getWorldSlotSizeTest){
    TestableSystemSettings::_worldSlotSize = 14;

    int output = 0;
    const bool success = ScriptTestHelper::executeStringInt("return _settings.getWorldSlotSize();", &output);

    //Restore the default before asserting, so a failure here can't leak the slot size into other tests.
    TestableSystemSettings::_worldSlotSize = 100;

    ASSERT_TRUE(success);
    ASSERT_EQ(output, 14);
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
        TestableSystemSettings::mCurrentRenderSystem = p.first;
        
        bool output = false;
        ASSERT_TRUE(ScriptTestHelper::executeStringBool("return _settings.getCurrentRenderSystem() == " + p.second, &output));
        
        ASSERT_TRUE(output);
    }
}
