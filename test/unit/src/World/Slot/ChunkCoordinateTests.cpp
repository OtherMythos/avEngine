#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "World/WorldSingleton.h"
#include "World/Slot/ChunkCoordinate.h"

TEST(ChunkCoordinateTests, getCoordsStringReturnsNullWhenOutOfBounds){
    AV::ChunkCoordinate coord1(-100000000, 0, "");

    ASSERT_EQ(coord1.getCoordsString(), "null");

    //--
    AV::ChunkCoordinate coord2(100000000, 0, "");

    ASSERT_EQ(coord2.getCoordsString(), "null");

    //--
    AV::ChunkCoordinate coord3(0, 100000000, "");

    ASSERT_EQ(coord3.getCoordsString(), "null");

    //--
    AV::ChunkCoordinate coord4(0, -100000000, "");

    ASSERT_EQ(coord4.getCoordsString(), "null");
}

TEST(ChunkCoordinateTests, getCoordsStringReturnsValidValueWithMaxChunk){
    AV::ChunkCoordinate coord1(AV::ChunkCoordinate::MAX_CHUNK, AV::ChunkCoordinate::MAX_CHUNK, "");

    ASSERT_EQ(coord1.getCoordsString(), std::to_string(AV::ChunkCoordinate::MAX_CHUNK) + std::to_string(AV::ChunkCoordinate::MAX_CHUNK));
}

TEST(ChunkCoordinateTests, getCoordsStringReturnsInValidValueAfterExceedingMaxChunk){
    AV::ChunkCoordinate coord1(AV::ChunkCoordinate::MAX_CHUNK + 1, AV::ChunkCoordinate::MAX_CHUNK + 1, "");

    ASSERT_EQ(coord1.getCoordsString(), "null");
}
