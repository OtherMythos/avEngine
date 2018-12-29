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
};

TEST(SystemSetupTests, SetupWindowTitle){
    const std::string key = "WindowTitle";
    const std::string testValue = "testValue";
    
    SystemSetupMock::processSettingsFileEntryExp(key, testValue);
    
    ASSERT_EQ(testValue, AV::SystemSettings::getWindowTitleSetting());
}

TEST(SystemSetupTests, SetupDataDirectory){
    const std::string key = "DataDirectory";
    const std::string testValue = "testValue";
    
    SystemSetupMock::processSettingsFileEntryExp(key, testValue);
    
    ASSERT_EQ(testValue, AV::SystemSettings::getDataPath());
}

TEST(SystemSetupTests, SetupCompositorBackground){
    const std::string key = "CompositorBackground";
    const std::string testValue = "1 0 1 1";
    
    SystemSetupMock::processSettingsFileEntryExp(key, testValue);
    
    Ogre::ColourValue value1 = AV::SystemSettings::getCompositorColourValue();
    Ogre::ColourValue value2 = Ogre::StringConverter::parseColourValue(testValue);
    
    ASSERT_EQ(value1, value2);
}

TEST(SystemSetupTests, SetupResourcesFile){
    const std::string key = "ResourcesFile";
    const std::string testValue = "testValue";
    
    SystemSetupMock::processSettingsFileEntryExp(key, testValue);
    
    ASSERT_EQ(testValue, AV::SystemSettings::_dataPath);
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
    SystemSetupMock::findOgreResourcesFile("/tmp/someRandomFileWhichShouldntExist.file");
    ASSERT_EQ(false, AV::SystemSettings::isSquirrelEntryScriptViable());
    
    SystemSetupMock::findOgreResourcesFile("/tmp");
    ASSERT_EQ(true, AV::SystemSettings::isSquirrelEntryScriptViable());
}
