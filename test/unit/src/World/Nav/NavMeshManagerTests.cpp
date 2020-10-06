#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include <fstream>
#include "World/Nav/NavMeshManager.h"

class NavMeshManagerTests : public ::testing::Test {
private:
    AV::NavMeshManager manager;
public:
    NavMeshManagerTests() {

    }

    virtual ~NavMeshManagerTests() {

    }

};

TEST_F(NavMeshManagerTests, meshIdToValues){
    AV::uint32 idx, version;
    manager._meshIdToValues(0, &idx, &version);
    ASSERT_EQ(idx, 0);
    ASSERT_EQ(version, 0);

    manager._meshIdToValues(1, &idx, &version);
    ASSERT_EQ(idx, 0);
    ASSERT_EQ(version, 1);

    AV::NavMeshId test = (AV::NavMeshId(10) << 32) + 3;
    manager._meshIdToValues(test, &idx, &version);
    ASSERT_EQ(idx, 10);
    ASSERT_EQ(version, 3);
}

TEST_F(NavMeshManagerTests, valuesToMeshId){
    AV::NavMeshId id = manager._valuesToMeshId(10, 3);
    AV::NavMeshId expected = (AV::NavMeshId(10) << 32) + 3;
    ASSERT_EQ(id, expected);

    id = manager._valuesToMeshId(20, 150);
    expected = (AV::NavMeshId(20) << 32) + 150;
    ASSERT_EQ(id, expected);
}

TEST_F(NavMeshManagerTests, valueToMeshReverse){
    AV::NavMeshId id = manager._valuesToMeshId(100, 300);

    AV::uint32 idx, version;
    manager._meshIdToValues(id, &idx, &version);
    ASSERT_EQ(idx, 100);
    ASSERT_EQ(version, 300);
}
