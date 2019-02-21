#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "System/SystemSetup/SystemSetup.h"
#include "System/SystemSetup/SystemSettings.h"

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

//TODO bring this back when you can (see the comment in the SystemSettings class).
TEST(SystemSetupTests, DISABLED_SetupResourcesFile){
    const std::string key = "ResourcesFile";
    const std::string testValue = "testValue";

    AV::SystemSettings::_dataPath = "/tmp/";

    SystemSetupMock::processSettingsFileEntryExp(key, testValue);

    ASSERT_EQ(testValue, AV::SystemSettings::getOgreResourceFilePath());
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

    SystemSetupMock::findOgreResourcesFile("/tmp");
    ASSERT_EQ(true, AV::SystemSettings::isOgreResourcesFileViable());
}

TEST(SystemSetupTests, SetupFindSquirrelEntryFile){
    SystemSetupMock::findSquirrelEntryFile("/tmp/someRandomFileWhichShouldntExist.file");
    ASSERT_EQ(false, AV::SystemSettings::isSquirrelEntryScriptViable());

    SystemSetupMock::findSquirrelEntryFile("/tmp");
    ASSERT_EQ(true, AV::SystemSettings::isSquirrelEntryScriptViable());
}
