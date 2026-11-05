#include "gtest/gtest.h"

#include <memory>
#include <vector>

#include "Threading/Thread/Physics/DynamicsWorldThreadLogic.h"
#include "btBulletDynamicsCommon.h"

using namespace AV;

/*
Covers which contacts count as a hit, against a real bullet world rather than a fake, as what's
under test is what bullet actually leaves in its manifolds after a step.

Bodies are given no motion state, which keeps them out of the transform output these tests have no
interest in. Nothing drains the impact buffer here, that being the main thread's job.
*/

namespace {
    const float STEP = 1.0f / 60.0f;
    const float THRESHOLD = 0.5f;
    const float BLOCK_HALF = 0.12f;
    const float BLOCK_MASS = 0.15f;

    class DynamicsImpactFixture : public ::testing::Test{
    protected:
        DynamicsWorldThreadLogic logic;
        btBoxShape floorShape{btVector3(5.0f, 0.5f, 5.0f)};
        btBoxShape blockShape{btVector3(BLOCK_HALF, BLOCK_HALF, BLOCK_HALF)};
        std::vector<std::unique_ptr<btRigidBody>> bodies;
        btRigidBody* floor = 0;

        void SetUp() override{
            logic.checkWorldConstructDestruct(true, 1);
            //The top of the floor is at y 0.
            floor = addBody(&floorShape, 0.0f, _at(btVector3(0.0f, -0.5f, 0.0f)));
        }

        void TearDown() override{
            logic.checkWorldConstructDestruct(false, 1);
        }

        btTransform _at(const btVector3& pos){
            btTransform t;
            t.setIdentity();
            t.setOrigin(pos);
            return t;
        }

        btRigidBody* addBody(btCollisionShape* shape, float mass, const btTransform& transform){
            btVector3 inertia(0, 0, 0);
            if(mass != 0.0f) shape->calculateLocalInertia(mass, inertia);

            btRigidBody::btRigidBodyConstructionInfo info(mass, 0, shape, inertia);
            info.m_startWorldTransform = transform;
            info.m_friction = 0.6f;
            bodies.push_back(std::make_unique<btRigidBody>(info));

            btRigidBody* body = bodies.back().get();
            logic.inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_ADD_BODY, body});
            return body;
        }

        //Height being how far off the floor its underside starts.
        btRigidBody* dropBlock(float height, const btVector3& offset = btVector3(0, 0, 0)){
            return addBody(&blockShape, BLOCK_MASS, _at(offset + btVector3(0.0f, BLOCK_HALF + height, 0.0f)));
        }

        void step(int count){
            for(int i = 0; i < count; i++) logic.updateWorld(STEP);
        }

        const std::vector<DynamicsWorldThreadLogic::OutputImpactEntry>& impacts() const{
            return logic.outputImpactBuffer;
        }
    };
}

TEST_F(DynamicsImpactFixture, nothingIsReportedUntilAThresholdIsSet){
    dropBlock(0.5f);

    step(60);

    EXPECT_TRUE(impacts().empty());
}

TEST_F(DynamicsImpactFixture, aFlatDropIsOneImpactAtAboutItsSpeed){
    logic.impactThreshold = THRESHOLD;
    btRigidBody* block = dropBlock(0.5f);

    step(60);

    ASSERT_EQ(impacts().size(), 1u);
    const DynamicsWorldThreadLogic::OutputImpactEntry& impact = impacts()[0];
    //3.1 falling half a unit, and a little more for what the solver does about the step's gravity and overlap.
    EXPECT_NEAR(impact.strength, 3.47f, 0.2f);
    //A contact is only found once the body has sunk in, which can be as much as a step's travel.
    EXPECT_NEAR(impact.pos.y(), 0.0f, 3.2f * STEP);
    EXPECT_TRUE((impact.bodyA == floor && impact.bodyB == block) || (impact.bodyA == block && impact.bodyB == floor));
}

TEST_F(DynamicsImpactFixture, aCornerFirstDropHitsMoreThanOnce){
    logic.impactThreshold = THRESHOLD;
    btTransform transform = _at(btVector3(0.0f, 0.7f, 0.0f));
    transform.setRotation(btQuaternion(btVector3(1, 0, 1).normalized(), 0.61f));
    addBody(&blockShape, BLOCK_MASS, transform);

    step(60);

    EXPECT_GT(impacts().size(), 1u);
}

TEST_F(DynamicsImpactFixture, aRestingBodyReportsNothingEvenAtZero){
    dropBlock(0.0f);
    step(10);

    logic.impactThreshold = 0.0f;
    step(30);

    EXPECT_TRUE(impacts().empty());
}

TEST_F(DynamicsImpactFixture, slidingMakesNewPointsButIsNotAHit){
    btRigidBody* block = dropBlock(0.0f);
    step(10);

    logic.impactThreshold = THRESHOLD;
    block->setLinearVelocity(btVector3(3.0f, 0.0f, 0.0f));
    step(60);

    EXPECT_TRUE(impacts().empty());
}

TEST_F(DynamicsImpactFixture, strengthIsTheLighterBodysChangeNotBothAddedUp){
    logic.impactThreshold = THRESHOLD;
    btRigidBody* under = dropBlock(0.0f);
    btRigidBody* top = dropBlock(0.4f + BLOCK_HALF * 2.0f);

    step(60);

    ASSERT_EQ(impacts().size(), 1u);
    const DynamicsWorldThreadLogic::OutputImpactEntry& impact = impacts()[0];
    EXPECT_TRUE((impact.bodyA == under && impact.bodyB == top) || (impact.bodyA == top && impact.bodyB == under));
    //Falling at 2.8. Adding the two inverse masses together reads twice this.
    EXPECT_NEAR(impact.strength, 3.2f, 0.4f);
}

TEST_F(DynamicsImpactFixture, impactsBuildUpAcrossSteps){
    logic.impactThreshold = THRESHOLD;
    dropBlock(0.2f, btVector3(-1.0f, 0.0f, 0.0f));
    dropBlock(0.5f, btVector3(1.0f, 0.0f, 0.0f));

    step(60);

    //They land on different steps, and an update can run several of those before anything reads the buffer.
    EXPECT_EQ(impacts().size(), 2u);
}

TEST_F(DynamicsImpactFixture, aPairWhichFallsAsleepDoesNotKeepReportingItsLastHit){
    logic.impactThreshold = THRESHOLD;
    btRigidBody* block = dropBlock(0.5f);
    while(impacts().empty()) step(1);

    //A manifold is left alone while its bodies sleep, so this point stays new with its impulse intact.
    block->forceActivationState(ISLAND_SLEEPING);
    logic.outputImpactBuffer.clear();
    step(5);

    EXPECT_TRUE(impacts().empty());
}
