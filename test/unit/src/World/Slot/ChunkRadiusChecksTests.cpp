#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

// #include "World/Slot/ChunkRadiusChecks.h"
//
// TEST(ChunkRadiusChecksTests, rectCircleCollisionTests){
//     //Sensible val close to the origin
//     bool val = AV::ChunkRadiusChecks::_checkRectCircleCollision(0, 0, 100, 10, 0, 0);
//     ASSERT_TRUE(val);
//
//     //Checking a tile of tile at 10 against the origin.
//     val = AV::ChunkRadiusChecks::_checkRectCircleCollision(10, 10, 100, 10, 0, 0);
//     ASSERT_FALSE(val);
// }
