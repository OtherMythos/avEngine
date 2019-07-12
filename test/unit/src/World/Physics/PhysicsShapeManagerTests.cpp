#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "World/Physics/PhysicsShapeManager.h"
#include "btBulletDynamicsCommon.h"

class PhysicsShapeManagerTests : public ::testing::Test {

    protected:
    btCollisionShape *testShape;

    PhysicsShapeManagerTests() {
    }

    virtual ~PhysicsShapeManagerTests() {
    }

    virtual void SetUp() {
        testShape = new btBoxShape(btVector3(1, 1, 1));
        void* shapePtr = reinterpret_cast<void*>((int)AV::PhysicsShapeManager::PhysicsShapeType::CubeShape);
        testShape->setUserPointer(shapePtr);

        for(int i = 0; i < 10; i++){
            AV::PhysicsShapeManager::mShapeMap[AV::PhysicsShapeManager::PhysicsShapeType::CubeShape].second.push_back(
                //Populate the shape map with some dummy entries.
                AV::PhysicsShapeManager::ShapeEntry(btVector3(i * 10, i * 10, i * 10), std::weak_ptr<btCollisionShape>())
            );
        }
    }

    virtual void TearDown() {
        delete testShape;

        AV::PhysicsShapeManager::shutdown();
    }

    void cleanListState(AV::PhysicsShapeManager::PhysicsShapeType shapeType = AV::PhysicsShapeManager::PhysicsShapeType::CubeShape){
        //If one of the tests doesn't want the intial values in the vector then clear them.
        AV::PhysicsShapeManager::mShapeMap[shapeType].second.clear();
    }

    void checkEntryIsHole(int holeIndex){
        auto& entry = AV::PhysicsShapeManager::mShapeMap[AV::PhysicsShapeManager::PhysicsShapeType::CubeShape].second[holeIndex];
        ASSERT_EQ(entry.first.x(), -1);
    }

    void assertHolePointsToIndex(int holeIndex, int targetIndex){
        auto& entry = AV::PhysicsShapeManager::mShapeMap[AV::PhysicsShapeManager::PhysicsShapeType::CubeShape].second[holeIndex];

        //Check that this is actually a hole.
        checkEntryIsHole(holeIndex);

        ASSERT_EQ(entry.first.y(), targetIndex);
    }

    void setShapeVectorIndex(int index, btCollisionShape* targetShape){
        testShape->setUserIndex(index);
    }

    void createHole(int index, int nextPoint){
        auto& entry = AV::PhysicsShapeManager::mShapeMap[AV::PhysicsShapeManager::PhysicsShapeType::CubeShape].second[index];

        entry.first = btVector3(-1, nextPoint, 0);
    }

    void assertFinalHole(int index){
        auto& entry = AV::PhysicsShapeManager::mShapeMap[AV::PhysicsShapeManager::PhysicsShapeType::CubeShape].second[index];

        ASSERT_EQ(entry.first.x(), -1);
        ASSERT_EQ(entry.first.y(), -1);
    }

    void setFirstArrayHole(int hole, AV::PhysicsShapeManager::PhysicsShapeType shapeType = AV::PhysicsShapeManager::PhysicsShapeType::CubeShape){
        AV::PhysicsShapeManager::mShapeMap[shapeType].first = hole;
    }

    void assertFirstArrayHole(int hole, AV::PhysicsShapeManager::PhysicsShapeType shapeType = AV::PhysicsShapeManager::PhysicsShapeType::CubeShape){
        ASSERT_EQ(hole, AV::PhysicsShapeManager::mShapeMap[shapeType].first);
    }

    int getShapeVectorSize(AV::PhysicsShapeManager::PhysicsShapeType shapeType = AV::PhysicsShapeManager::PhysicsShapeType::CubeShape){
        return AV::PhysicsShapeManager::mShapeMap[shapeType].second.size();
    }
};

TEST_F(PhysicsShapeManagerTests, noFirstHoleCausesFirstHoleCreation){
    //Say that this shape is in the first index of the list.
    //When it's destroyed the first hole should become this same index.
    setFirstArrayHole(-1);

    setShapeVectorIndex(0, testShape);

    AV::PhysicsShapeManager::_removeShape(testShape);

    assertFirstArrayHole(0);

    assertFinalHole(0);
}

TEST_F(PhysicsShapeManagerTests, lessThanFirstHoleCausesNewFirst){
    //If we already have a hole in the array, and ask to delete a shape which is less than that hole, that newly deleted index should become the new first hole.
    //The new first hole should point to the previous first.
    setFirstArrayHole(5);

    //Tell the shape about to be deleted it lives at index 2 in the vector.
    setShapeVectorIndex(2, testShape);

    AV::PhysicsShapeManager::_removeShape(testShape);

    //The new first hole should be 2.
    assertFirstArrayHole(2);

    //The new first hole should point to what was originally the first.
    assertHolePointsToIndex(2, 5);
}

TEST_F(PhysicsShapeManagerTests, greaterThanFirstCausesReArrange){
    //Here I have two holes [ ;) ]. It looks like this:
    // 0 F 0 T 0 H 0
    //F means first hole, T is target and H is just a regular hole.
    //Where T was previously a shape, I delete it.
    //I expect F which previously pointed to H to now point to T, and T to point to H.

    setFirstArrayHole(1);

    //Make F and H be holes.
    createHole(1, 5); //F
    createHole(5, -1); //H, final hole so -1

    //Tell T it lives at 3.
    setShapeVectorIndex(3, testShape);

    AV::PhysicsShapeManager::_removeShape(testShape);

    //The first hole should still be F's index.
    assertFirstArrayHole(1);

    assertHolePointsToIndex(1, 3); //F to T
    assertHolePointsToIndex(3, 5); //T to H
}

TEST_F(PhysicsShapeManagerTests, deleteAfterFinalHole){
    // 0 F 0 H H T
    setFirstArrayHole(1);

    createHole(1, 3); //F
    createHole(3, 4); //H1
    createHole(4, -1); //H2, final hole

    //T is 5
    setShapeVectorIndex(5, testShape);

    AV::PhysicsShapeManager::_removeShape(testShape);

    //Check the first hole wasn't changed.
    assertFirstArrayHole(1);

    //5 should be the new final hole.
    assertHolePointsToIndex(5, -1);
}

TEST_F(PhysicsShapeManagerTests, determineListPositionReturnsMinusOneOnNoHole){
    int holeVal = -1;
    int result = AV::PhysicsShapeManager::_determineListPosition(AV::PhysicsShapeManager::mShapeMap[AV::PhysicsShapeManager::PhysicsShapeType::CubeShape].second, holeVal);

    ASSERT_EQ(result, -1);
}

TEST_F(PhysicsShapeManagerTests, determineListPositionReturnsIndexWithHole){
    setFirstArrayHole(1);

    createHole(1, 3); //Start at one, and it thinks the next hole is 3.

    int firstHole = 1;
    int result = AV::PhysicsShapeManager::_determineListPosition(AV::PhysicsShapeManager::mShapeMap[AV::PhysicsShapeManager::PhysicsShapeType::CubeShape].second, firstHole);

    //It should tell us to insert into 1, and the firstHole variable should then become 3
    ASSERT_EQ(result, 1);
    ASSERT_EQ(firstHole, 3);
}

TEST_F(PhysicsShapeManagerTests, getBoxShapeCreatesShapes){
    cleanListState();
    {
        AV::PhysicsShapeManager::ShapePtr first = AV::PhysicsShapeManager::getBoxShape(btVector3(10, 20, 30));

        ASSERT_EQ(AV::PhysicsShapeManager::getBoxShape(btVector3(10, 20, 30)), first);

        //Even though the function was called twice, only the single shape should have been created.
        ASSERT_EQ(getShapeVectorSize(), 1);

        ASSERT_NE(AV::PhysicsShapeManager::getBoxShape(btVector3(10, 20, 40)), first);

        //Now another should have appeared.
        ASSERT_EQ(getShapeVectorSize(), 2);
    }
    //Outside of the scope the shapes should be destroyed.
    //There should now be a hole at the beginning of the vector.
    assertFirstArrayHole(0);

    checkEntryIsHole(0);
    checkEntryIsHole(1);
}

TEST_F(PhysicsShapeManagerTests, getShapeOfDifferentTypes){
    cleanListState();
    {
        AV::PhysicsShapeManager::ShapePtr box = AV::PhysicsShapeManager::getBoxShape(btVector3(10, 20, 30));

        AV::PhysicsShapeManager::ShapePtr firstSphere = AV::PhysicsShapeManager::getSphereShape(10);
        ASSERT_NE(firstSphere, box);

        ASSERT_EQ(AV::PhysicsShapeManager::getSphereShape(10), firstSphere);
    }
}
