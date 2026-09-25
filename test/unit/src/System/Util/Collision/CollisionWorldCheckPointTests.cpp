#include "gtest/gtest.h"

#include "System/Util/Collision/CollisionWorldOctree.h"
#include "System/Util/Collision/CollisionWorldBruteForce.h"

#include <cmath>
#include <memory>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//checkCollisionPoint tests a CIRCLE of the given radius, not a point, against every
//shape. Run against both world types so they have to agree.
class CollisionWorldCheckPointTests : public ::testing::TestWithParam<bool> {
protected:
    std::unique_ptr<AV::CollisionWorldObject> world;

    virtual void SetUp() {
        if(GetParam()) world.reset(new AV::CollisionWorldOctree(0));
        else world.reset(new AV::CollisionWorldBruteForce(0));
    }
};

TEST_P(CollisionWorldCheckPointTests, radiusReachesCircle){
    world->addCollisionPoint(0, 0, 1);

    //1.2 from the centre: clear of the circle by 0.2.
    ASSERT_FALSE(world->checkCollisionPoint(1.2f, 0, 0.0f));
    ASSERT_FALSE(world->checkCollisionPoint(1.2f, 0, 0.1f));
    ASSERT_TRUE(world->checkCollisionPoint(1.2f, 0, 0.3f));
}

TEST_P(CollisionWorldCheckPointTests, radiusReachesRectangle){
    //4 wide, 2 high, centred on the origin: its right edge at x = 2.
    world->addCollisionRectangle(0, 0, 4, 2);

    ASSERT_TRUE(world->checkCollisionPoint(1.9f, 0, 0.0f));
    ASSERT_FALSE(world->checkCollisionPoint(2.5f, 0, 0.0f));
    ASSERT_FALSE(world->checkCollisionPoint(2.5f, 0, 0.4f));
    ASSERT_TRUE(world->checkCollisionPoint(2.5f, 0, 0.6f));
}

TEST_P(CollisionWorldCheckPointTests, radiusReachesRotatedRectangle){
    //The same rectangle turned a quarter turn: its long side now runs along y, so
    //its edge is at x = 1 and its end at y = 2.
    world->addCollisionRotatedRectangle(10, 10, 4, 2, M_PI * 0.5f);

    ASSERT_TRUE(world->checkCollisionPoint(10, 11.9f, 0.0f));
    ASSERT_FALSE(world->checkCollisionPoint(11.5f, 10, 0.0f));
    ASSERT_FALSE(world->checkCollisionPoint(11.5f, 10, 0.4f));
    ASSERT_TRUE(world->checkCollisionPoint(11.5f, 10, 0.6f));
    ASSERT_FALSE(world->checkCollisionPoint(10, 12.5f, 0.4f));
    ASSERT_TRUE(world->checkCollisionPoint(10, 12.5f, 0.6f));
}

TEST_P(CollisionWorldCheckPointTests, radiusReachesAcrossOctreeCells){
    //Shapes far apart spread the octree over several cells. A query sitting just
    //outside one shape's bounds has to find it through its radius alone.
    for(int i = 0; i < 40; i++){
        world->addCollisionPoint(-900.0f + i * 45.0f, -900.0f + i * 45.0f, 0.5f);
    }
    world->addCollisionRectangle(300, 300, 2, 2);

    ASSERT_FALSE(world->checkCollisionPoint(301.5f, 300, 0.4f));
    ASSERT_TRUE(world->checkCollisionPoint(301.5f, 300, 0.6f));
}

INSTANTIATE_TEST_SUITE_P(WorldTypes, CollisionWorldCheckPointTests, ::testing::Values(true, false),
    [](const ::testing::TestParamInfo<bool>& info){ return info.param ? "Octree" : "BruteForce"; });
