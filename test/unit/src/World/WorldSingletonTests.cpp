#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "World/WorldSingleton.h"
#include "unit/src/TestAccessors.h"

class WorldSingletonTests : public ::testing::Test {
private:
public:
    WorldSingletonTests() {
    }

    virtual ~WorldSingletonTests() {
    }

    virtual void SetUp() {
        //reset the singleton.
        TestableWorldSingleton::_world = 0;
        TestableWorldSingleton::_origin = AV::SlotPosition();
    }

    virtual void TearDown() {
    }
};

TEST_F(WorldSingletonTests, getOriginTest){
    const AV::SlotPosition pos = AV::SlotPosition(1, 1, Ogre::Vector3(100, 200, 300));

    TestableWorldSingleton::_origin = pos;

    ASSERT_EQ(AV::WorldSingleton::getOrigin(), pos);
}
