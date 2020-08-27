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

TEST_F(WorldSingletonTests, getOriginTest){
    const AV::SlotPosition pos = AV::SlotPosition(1, 1, Ogre::Vector3(100, 200, 300));

    AV::WorldSingleton::_origin = pos;

    ASSERT_EQ(AV::WorldSingleton::getOrigin(), pos);
}
