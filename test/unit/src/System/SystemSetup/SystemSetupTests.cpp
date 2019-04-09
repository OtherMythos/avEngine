#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "System/SystemSetup/SystemSetup.h"
#include "System/SystemSetup/SystemSettings.h"
#include "System/SystemSetup/UserSettings.h"

#include <OgreColourValue.h>
#include <OgreStringConverter.h>

class SystemSetupMock : public AV::SystemSetup{
public:
    static void processSettingsFileEntryExp(const Ogre::String &key, const Ogre::String &value){
        _processSettingsFileEntry(key, value);
    }

    static void findOgreResourcesFile(const std::string &path){
        _findOgreResourcesFile(path);
    }

    static void findSquirrelEntryFile(const std::string &path){
        _findSquirrelEntryFile(path);
    }

    static AV::SystemSettings::RenderSystemTypes parseRenderSystemString(const std::string &path){
        return _parseRenderSystemString(path);
    }

    static AV::SystemSettings::RenderSystemTypes determineRenderSystem(){
        return _determineRenderSystem();
    }
};

TEST(SystemSetupTests, SetupWindowTitle){
    const std::string key = "WindowTitle";
    const std::string testValue = "testValue";

    SystemSetupMock::processSettingsFileEntryExp(key, testValue);

    ASSERT_EQ(testValue, AV::SystemSettings::getWindowTitleSetting());
}

TEST(SystemSetupTests, SetupDataDirectoryAbsolutePath){
    const std::string key = "DataDirectory";
    const std::string testValue = "/tmp/";

    SystemSetupMock::processSettingsFileEntryExp(key, testValue);

    ASSERT_EQ(testValue, AV::SystemSettings::getDataPath());
}

TEST(SystemSetupTests, SetupDataDirectoryRelativePath){
    const std::string key = "DataDirectory";
    const std::string testValue = "../";

    std::string beforeFilePath = AV::SystemSettings::getAvSetupFilePath();
    AV::SystemSettings::_avSetupFilePath = "/tmp/";

    SystemSetupMock::processSettingsFileEntryExp(key, testValue);

    ASSERT_EQ(AV::SystemSettings::getDataPath(), "/");

    //Reset the state.
    AV::SystemSettings::_avSetupFilePath = beforeFilePath;
}

TEST(SystemSetupTests, SetupCompositorBackground){
    const std::string key = "CompositorBackground";
    const std::string testValue = "1 0 1 1";

    SystemSetupMock::processSettingsFileEntryExp(key, testValue);

    Ogre::ColourValue value1 = AV::SystemSettings::getCompositorColourValue();
    Ogre::ColourValue value2 = Ogre::StringConverter::parseColourValue(testValue);

    ASSERT_EQ(value1, value2);
}

TEST(SystemSetupTests, SetupSquirrelEntry){
    const std::string key = "SquirrelEntryFile";
    const std::string testValue = "testValue";

    SystemSetupMock::processSettingsFileEntryExp(key, testValue);

    ASSERT_EQ(testValue, AV::SystemSettings::_squirrelEntryScriptPath);
}

TEST(SystemSetupTests, SetupFindOgreResourcesFile){
    SystemSetupMock::findOgreResourcesFile("/tmp/someRandomFileWhichShouldntExist.file");
    ASSERT_EQ(false, AV::SystemSettings::isOgreResourcesFileViable());

    //This is a complete path, but tmp is a directory. It wants a file.
    SystemSetupMock::findOgreResourcesFile("/tmp");
    ASSERT_EQ(false, AV::SystemSettings::isOgreResourcesFileViable());
}

TEST(SystemSetupTests, SetupFindSquirrelEntryFile){
    SystemSetupMock::findSquirrelEntryFile("/tmp/someRandomFileWhichShouldntExist.file");
    ASSERT_EQ(false, AV::SystemSettings::isSquirrelEntryScriptViable());

    SystemSetupMock::findSquirrelEntryFile("/tmp");
    ASSERT_EQ(false, AV::SystemSettings::isSquirrelEntryScriptViable());
}

TEST(SystemSetupTests, ParseRenderSystemStringReturnsCorrectValues){
    //Nothing on bad value.
    AV::SystemSettings::RenderSystemTypes result = SystemSetupMock::parseRenderSystemString("");
    ASSERT_EQ(AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_UNSET, result);

    result = SystemSetupMock::parseRenderSystemString("djskjfalksdjlk");
    ASSERT_EQ(AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_UNSET, result);

    //Correct values.
    result = SystemSetupMock::parseRenderSystemString("Metal");
    ASSERT_EQ(AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL, result);

    result = SystemSetupMock::parseRenderSystemString("OpenGL");
    ASSERT_EQ(AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL, result);

    result = SystemSetupMock::parseRenderSystemString("Direct3D11");
    ASSERT_EQ(AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_D3D11, result);
}

TEST(SystemSetupTests, DetermineRenderSystemReturnsUnsetOnEmptyList){
    //Check it returns unset on empty.
    AV::SystemSettings::mAvailableRenderSystems = {};

    AV::SystemSettings::RenderSystemTypes result = SystemSetupMock::determineRenderSystem();
    ASSERT_EQ(AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_UNSET, result);
}

TEST(SystemSetupTests, DetermineRenderSystemReturnsValueOnList){
    //Should return the first value in the list.
    AV::SystemSettings::mAvailableRenderSystems = {
        AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL,
        AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL
    };

    AV::SystemSettings::RenderSystemTypes result = SystemSetupMock::determineRenderSystem();
    ASSERT_EQ(AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL, result);

    //Should still return the first value.
    AV::SystemSettings::mAvailableRenderSystems = {
        AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL,
        AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL
    };

    result = SystemSetupMock::determineRenderSystem();
    ASSERT_EQ(AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL, result);
}

TEST(SystemSetupTests, DetermineRenderSystemReturnsRequestedType){
    AV::UserSettings::mRequestedRenderSystem = "OpenGL";

    AV::SystemSettings::mAvailableRenderSystems = {
        AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL,
        AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL
    };

    AV::SystemSettings::RenderSystemTypes result = SystemSetupMock::determineRenderSystem();
    ASSERT_EQ(AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL, result);

    //Even though metal is now the default, we should still get opengl.
    AV::SystemSettings::mAvailableRenderSystems = {
        AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL,
        AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL
    };

    result = SystemSetupMock::determineRenderSystem();
    ASSERT_EQ(AV::SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL, result);
}
