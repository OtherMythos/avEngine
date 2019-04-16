#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "../ScriptTestHelper.h"

#define private public

#include "World/WorldSingleton.h"
#include "System/SystemSetup/SystemSettings.h"

TEST(WorldNamespaceTests, getPlayerLoadRadiusTest){
    AV::WorldSingleton::mPlayerLoadRadius = 130;
    
    int output = 0;
    ASSERT_TRUE(ScriptTestHelper::executeStringInt("return _world.getPlayerLoadRadius();", &output));
    
    ASSERT_EQ(output, 130);
}

TEST(WorldNamespaceTests, setPlayerLoadRadiusTest){
    AV::WorldSingleton::mPlayerLoadRadius = 130;
    
    ScriptTestHelper::executeString("_world.setPlayerLoadRadius(10);");
    
    ASSERT_EQ(AV::WorldSingleton::mPlayerLoadRadius, 10);
}

TEST(WorldNamespaceTests, getPlayerPositionTest){
    //Needs to be big enough to store these values.
    AV::SystemSettings::_worldSlotSize = 100;
    AV::WorldSingleton::_playerPosition = AV::SlotPosition(1, 2, Ogre::Vector3(10, 20, 30));
    
    bool output = false;
    std::string s =
    "local pos = _world.getPlayerPosition();"
    "return pos.x == 10 && pos.y == 20 && pos.z == 30 && pos.slotX == 1 && pos.slotY == 2;";
    ASSERT_TRUE(ScriptTestHelper::executeStringBool(s, &output));
    
    ASSERT_TRUE(output);
}

TEST(WorldNamespaceTests, setPlayerPositionTest){
    AV::WorldSingleton::_playerPosition = AV::SlotPosition(3, 4, Ogre::Vector3(50, 60, 70));
    
    std::string s =
    "local s = SlotPosition(1, 2, 10, 20, 30);"
    "_world.setPlayerPosition(s);";
    ScriptTestHelper::executeString(s);
    
    ASSERT_EQ(AV::WorldSingleton::getPlayerPosition(), AV::SlotPosition(1, 2, Ogre::Vector3(10, 20, 30)));
}
