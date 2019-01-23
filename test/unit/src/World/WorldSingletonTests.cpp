#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "World/WorldSingleton.h"

void resetSingleton(){
    AV::WorldSingleton::_world = 0;
    AV::WorldSingleton::_origin = AV::SlotPosition();
}

TEST(WorldSingletonTests, DISABLED_createWorldTest){
    resetSingleton();

    bool val = AV::WorldSingleton::createWorld();

    ASSERT_TRUE(val);

    val = AV::WorldSingleton::createWorld();

    ASSERT_FALSE(val);

    //Cleanup
    val = AV::WorldSingleton::destroyWorld();
}

TEST(WorldSingletonTests, DISABLED_destroyWorldTest){
    resetSingleton();

    bool val = AV::WorldSingleton::destroyWorld();

    ASSERT_FALSE(val);

    AV::WorldSingleton::createWorld();
    val = AV::WorldSingleton::destroyWorld();

    ASSERT_TRUE(val);
}

TEST(WorldSingletonTests, getOriginTest){
    resetSingleton();
    const AV::SlotPosition pos = AV::SlotPosition(1, 1, Ogre::Vector3(100, 200, 300));

    AV::WorldSingleton::_origin = pos;

    ASSERT_EQ(AV::WorldSingleton::getOrigin(), pos);
}

TEST(WorldSingletonTests, DISABLED_getWorldTests){
    resetSingleton();

    ASSERT_EQ(AV::WorldSingleton::getWorld(), (void*)0);

    AV::WorldSingleton::createWorld();
    ASSERT_NE(AV::WorldSingleton::getWorld(), (void*)0);

    AV::WorldSingleton::destroyWorld();
    ASSERT_EQ(AV::WorldSingleton::getWorld(), (void*)0);
}
