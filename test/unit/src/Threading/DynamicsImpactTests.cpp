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

TEST_F(DynamicsImpactFixture, shiftsPreserveSleepingContactsWithoutNewImpacts){
    btRigidBody* block = dropBlock(0.0f);
    step(180);
    ASSERT_FALSE(block->isActive());
    logic.impactThreshold = 0.0f;
    const btVector3 start = block->getWorldTransform().getOrigin();
    const btVector3 delta(128.0f, 0.0f, -256.0f);
    for(int i = 0; i < 8; i++){
        logic.inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_SHIFT_ORIGIN, nullptr, delta});
        step(1);
        EXPECT_FALSE(block->isActive());
        EXPECT_NEAR((block->getWorldTransform().getOrigin() + delta * (i + 1) - start).length(), 0.0f, 0.001f);
        EXPECT_TRUE(impacts().empty());
    }
    EXPECT_EQ(logic.statistics.shifts, 8u);
    EXPECT_EQ(logic.statistics.awakeBodies, 0);
    EXPECT_EQ(logic.statistics.staticBodies, 1);
}

TEST_F(DynamicsImpactFixture, shiftsPreserveVelocitySpinAndOrientation){
    const btQuaternion rotation(btVector3(1, 1, 0).normalized(), 0.7f);
    btTransform transform(rotation, btVector3(1, 4, 2));
    btRigidBody* block = addBody(&blockShape, BLOCK_MASS, transform);
    block->setLinearVelocity(btVector3(2, 0, -1));
    block->setAngularVelocity(btVector3(0, 1.5f, 0));
    const btVector3 delta(4096, 0, -8192);
    logic.inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_SHIFT_ORIGIN, nullptr, delta});
    step(1);
    EXPECT_NEAR(block->getLinearVelocity().x(), 2.0f, 0.0001f);
    EXPECT_NEAR(block->getAngularVelocity().y(), 1.5f, 0.0001f);
    EXPECT_NEAR(block->getWorldTransform().getOrigin().x() + delta.x(), 1.0f + 2.0f * STEP, 0.001f);
    EXPECT_LT(block->getOrientation().angleShortestPath(rotation), 0.03f);
    EXPECT_NEAR(floor->getWorldTransform().getOrigin().x(), -delta.x(), 0.001f);
}

TEST_F(DynamicsImpactFixture, shiftsAreOrderedWithAddsMovesAndRemovals){
    using Command = DynamicsWorldThreadLogic::ObjectCommandType;
    btRigidBody* before = dropBlock(4.0f);
    const btVector3 delta(128, 0, 256);
    logic.inputObjectCommandBuffer.push_back({Command::COMMAND_TYPE_SHIFT_ORIGIN, nullptr, delta});
    btRigidBody* after = dropBlock(5.0f);
    logic.inputObjectCommandBuffer.push_back({Command::COMMAND_TYPE_SET_POSITION, before, btVector3(3, 5, 4)});
    logic.inputObjectCommandBuffer.push_back({Command::COMMAND_TYPE_REMOVE_BODY, before});
    logic.inputObjectCommandBuffer.push_back({Command::COMMAND_TYPE_ADD_BODY, before});
    step(1);
    EXPECT_NEAR(before->getWorldTransform().getOrigin().x(), 3.0f, 0.001f);
    EXPECT_NEAR(after->getWorldTransform().getOrigin().x(), 0.0f, 0.001f);
    EXPECT_EQ(logic.statistics.dynamicBodies, 2);
    EXPECT_EQ(logic.statistics.staticBodies, 1);
}

TEST_F(DynamicsImpactFixture, undeliveredImpactsMoveWithTheOrigin){
    logic.impactThreshold = THRESHOLD;
    dropBlock(0.2f);
    step(30);
    ASSERT_FALSE(impacts().empty());
    const btVector3 position = impacts()[0].pos;
    const btVector3 delta(-128, 0, 256);
    logic.inputObjectCommandBuffer.push_back({DynamicsWorldThreadLogic::ObjectCommandType::COMMAND_TYPE_SHIFT_ORIGIN, nullptr, delta});
    step(1);
    EXPECT_NEAR((impacts()[0].pos + delta - position).length(), 0.0f, 0.001f);
}
