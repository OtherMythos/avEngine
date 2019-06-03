#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "World/Physics/PhysicsShapeManager.h"
#include "btBulletDynamicsCommon.h"

class PhysicsShapeManagerTests : public ::testing::Test {
        
    protected:
    std::unique_ptr<AV::PhysicsShapeManager> manager;
    btCollisionShape *testShape;
    
    PhysicsShapeManagerTests() {
    }
    
    virtual ~PhysicsShapeManagerTests() {
    }
    
    virtual void SetUp() {
        manager = std::unique_ptr<AV::PhysicsShapeManager>(new AV::PhysicsShapeManager());
        
        testShape = new btBoxShape(btVector3(1, 1, 1));
        void* shapePtr = reinterpret_cast<void*>((int)AV::PhysicsShapeManager::PhysicsShapeType::CubeShape);
        testShape->setUserPointer(shapePtr);
        
        for(int i = 0; i < 10; i++){
            manager->mShapeMap[AV::PhysicsShapeManager::PhysicsShapeType::CubeShape].push_back(
                //Populate the shape map with some dummy entries.
                AV::PhysicsShapeManager::ShapeEntry(btVector3(i * 10, i * 10, i * 10), std::weak_ptr<btCollisionShape>())
            );
        }
    }
    
    virtual void TearDown() {
        delete testShape;
    }
    
    void assertHolePointsToIndex(int holeIndex, int targetIndex){
        auto& entry = manager->mShapeMap[AV::PhysicsShapeManager::PhysicsShapeType::CubeShape][holeIndex];
        
        //Check that this is actually a hole.
        ASSERT_EQ(entry.first.x(), -1);
        
        ASSERT_EQ(entry.first.y(), targetIndex);
    }
    
    void setShapeVectorIndex(int index, btCollisionShape* targetShape){
        testShape->setUserIndex(index);
    }
    
    void createHole(int index, int nextPoint){
        auto& entry = manager->mShapeMap[AV::PhysicsShapeManager::PhysicsShapeType::CubeShape][index];
        
        entry.first = btVector3(-1, nextPoint, 0);
    }
    
    void assertFinalHole(int index){
        auto& entry = manager->mShapeMap[AV::PhysicsShapeManager::PhysicsShapeType::CubeShape][index];
        
        ASSERT_EQ(entry.first.x(), -1);
        ASSERT_EQ(entry.first.y(), -1);
    }
};

TEST_F(PhysicsShapeManagerTests, noFirstHoleCausesFirstHoleCreation){
    //Say that this shape is in the first index of the list.
    //When it's destroyed the first hole should become this same index.
    manager->mFirstArrayHole = -1;
    
    setShapeVectorIndex(0, testShape);
    
    AV::PhysicsShapeManager::_destroyShape(testShape);
    
    ASSERT_EQ(manager->mFirstArrayHole, 0);
    
    assertFinalHole(0);
}

TEST_F(PhysicsShapeManagerTests, lessThanFirstHoleCausesNewFirst){
    //If we already have a hole in the array, and ask to delete a shape which is less than that hole, that newly deleted index should become the new first hole.
    //The new first hole should point to the previous first.
    manager->mFirstArrayHole = 5;
    
    //Tell the shape about to be deleted it lives at index 2 in the vector.
    setShapeVectorIndex(2, testShape);
    
    AV::PhysicsShapeManager::_destroyShape(testShape);
    
    //The new first hole should be 2.
    ASSERT_EQ(manager->mFirstArrayHole, 2);
    
    //The new first hole should point to what was originally the first.
    assertHolePointsToIndex(2, 5);
}

TEST_F(PhysicsShapeManagerTests, greaterThanFirstCausesReArrange){
    //Here I have two holes [ ;) ]. It looks like this:
    // 0 F 0 T 0 H 0
    //F means first hole, T is target and H is just a regular hole.
    //Where T was previously a shape, I delete it.
    //I expect F which previously pointed to H to now point to T, and T to point to H.
    
    manager->mFirstArrayHole = 1;
    
    //Make F and H be holes.
    createHole(1, 5); //F
    createHole(5, -1); //H, final hole so -1
    
    //Tell T it lives at 3.
    setShapeVectorIndex(3, testShape);
    
    AV::PhysicsShapeManager::_destroyShape(testShape);
    
    //The first hole should still be F's index.
    ASSERT_EQ(manager->mFirstArrayHole, 1);
    
    assertHolePointsToIndex(1, 3); //F to T
    assertHolePointsToIndex(3, 5); //T to H
}

TEST_F(PhysicsShapeManagerTests, deleteAfterFinalHole){
    // 0 F 0 H H T
    manager->mFirstArrayHole = 1;
    
    createHole(1, 3); //F
    createHole(3, 4); //H1
    createHole(4, -1); //H2, final hole
    
    //T is 5
    setShapeVectorIndex(5, testShape);
    
    AV::PhysicsShapeManager::_destroyShape(testShape);
    
    //Check the first hole wasn't changed.
    ASSERT_EQ(manager->mFirstArrayHole, 1);
    
    //5 should be the new final hole.
    assertHolePointsToIndex(5, -1);
}
