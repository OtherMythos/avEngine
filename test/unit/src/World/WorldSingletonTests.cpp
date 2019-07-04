#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "World/WorldSingleton.h"

class WorldSingletonTests : public ::testing::Test {
private:
public:
    WorldSingletonTests() {
    }

    virtual ~WorldSingletonTests() {
    }

    virtual void SetUp() {
        //reset the singleton.
        AV::WorldSingleton::_world = 0;
        AV::WorldSingleton::_origin = AV::SlotPosition();
    }

    virtual void TearDown() {
    }
};

TEST_F(WorldSingletonTests, DISABLED_createWorldTest){
    bool val = AV::WorldSingleton::createWorld();

    ASSERT_TRUE(val);

    val = AV::WorldSingleton::createWorld();

    ASSERT_FALSE(val);

    //Cleanup
    val = AV::WorldSingleton::destroyWorld();
}

TEST_F(WorldSingletonTests, DISABLED_destroyWorldTest){
    bool val = AV::WorldSingleton::destroyWorld();

    ASSERT_FALSE(val);

    AV::WorldSingleton::createWorld();
    val = AV::WorldSingleton::destroyWorld();

    ASSERT_TRUE(val);
}

TEST_F(WorldSingletonTests, getOriginTest){
    const AV::SlotPosition pos = AV::SlotPosition(1, 1, Ogre::Vector3(100, 200, 300));

    AV::WorldSingleton::_origin = pos;

    ASSERT_EQ(AV::WorldSingleton::getOrigin(), pos);
}

TEST_F(WorldSingletonTests, DISABLED_getWorldTests){
    ASSERT_EQ(AV::WorldSingleton::getWorld(), (void*)0);

    AV::WorldSingleton::createWorld();
    ASSERT_NE(AV::WorldSingleton::getWorld(), (void*)0);

    AV::WorldSingleton::destroyWorld();
    ASSERT_EQ(AV::WorldSingleton::getWorld(), (void*)0);
}
