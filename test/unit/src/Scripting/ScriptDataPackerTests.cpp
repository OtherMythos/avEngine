#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "Scripting/ScriptDataPacker.h"

struct _testData{
    int first;
    float second;

    bool operator==(const _testData &d) const{
        return d.first == first && d.second == second;
    }
};

class ScriptDataPackerTests : public ::testing::Test{
protected:
    typedef std::unique_ptr<AV::ScriptDataPacker<_testData>> PtrType;
    PtrType packer;

    ScriptDataPackerTests(){

    }

    virtual ~ScriptDataPackerTests(){

    }

    virtual void SetUp(){
        packer = PtrType(new AV::ScriptDataPacker<_testData>());
    }

    virtual void TearDown(){

    }

    void setFirstHole(int firstHole){
        packer->mFirstHole = firstHole;
    }

    void setupEntries(){
        for(int i = 0; i < 10; i++){
            _testData d;
            d.first = i;
            d.second = i;
            packer->storeEntry(d);
        }

        ASSERT_EQ(packer->mFirstHole, -1);
    }

    void createHole(size_t index, int nextPoint){
        packer->mDataVec[index].second = nextPoint;
    }

    void assertHolePointsToIndex(size_t index, int nextPoint){
        ASSERT_EQ(packer->mDataVec[index].second, nextPoint);
    }

    size_t voidPtrToIndex(void* ptr){
        return reinterpret_cast<size_t>(ptr);
    }

    void* indexToPtr(size_t index){
        return reinterpret_cast<void*>(index);
    }
};

/*
This group of tests is very similar to the PhysicsShapeManagerTests.
The reason being that I implemented the first free-list there.
This is a very similar piece of code, so I've mostly just copied lots of stuff over.
*/

TEST_F(ScriptDataPackerTests, noFirstHoleCausesFirstHoleCreation){
    _testData e;
    void* value = packer->storeEntry(e);

    size_t index = voidPtrToIndex(value);

    ASSERT_EQ(index, 0);
}

TEST_F(ScriptDataPackerTests, lessThanFirstHoleCausesNewFirst){
    setupEntries();

    setFirstHole(5);

    void* destroyEntry = indexToPtr(2);

    packer->removeEntry(destroyEntry);

    ASSERT_EQ(packer->mFirstHole, 2);
}

TEST_F(ScriptDataPackerTests, greaterThanFirstCausesReArrange){
    setupEntries();

    setFirstHole(1);

    createHole(1, 5);
    createHole(5, -1);

    void* destroyEntry = indexToPtr(3);

    packer->removeEntry(destroyEntry);

    ASSERT_EQ(packer->mFirstHole, 1);

    assertHolePointsToIndex(1, 3);
    assertHolePointsToIndex(3, 5);
}

TEST_F(ScriptDataPackerTests, deleteAfterFinalHole){
    setupEntries();

    setFirstHole(1);

    createHole(1, 3);
    createHole(3, 4);
    createHole(4, -1);

    void* destroyEntry = indexToPtr(5);

    packer->removeEntry(destroyEntry);

    ASSERT_EQ(packer->mFirstHole, 1);
    assertHolePointsToIndex(5, -1);
}

TEST_F(ScriptDataPackerTests, createEntriesAndDestroy){
    //Create a number of entries, remove one, insert another and check the size was the same as before the first removal.
    setupEntries();

    //Remove the middle entry.
    void* destroyEntry = indexToPtr(5);
    packer->removeEntry(destroyEntry);
    ASSERT_EQ(packer->mFirstHole, 5);

    _testData e;
    e.first = 15;
    e.second = 1.234;
    //Check the entry was stored in the list.
    void* value = packer->storeEntry(e);
    ASSERT_EQ(voidPtrToIndex(value), 5);

    //Check that when trying to get that value it equals the one inserted.
    _testData& data = packer->getEntry(value);
    ASSERT_EQ(data, e);
}

TEST_F(ScriptDataPackerTests, listOnlyGrowsWhenNecessary){
    //Create lots of entries, delete a few, create a few more, check the size of the list hasn't grown any.
    setupEntries();

    ASSERT_EQ(packer->mDataVec.size(), 10);

    for(int i = 0; i < 3; i++){
        void* destroyEntry = indexToPtr(i * 2);
        packer->removeEntry(destroyEntry);
    }

    ASSERT_EQ(packer->mDataVec.size(), 10);

    for(int i = 0; i < 3; i++){
        _testData e;
        void* value = packer->storeEntry(e);
    }

    ASSERT_EQ(packer->mDataVec.size(), 10);

    //Now the list is free of holes, creating another entry should cause the list to grow.
    _testData e;
    void* value = packer->storeEntry(e);

    ASSERT_EQ(packer->mDataVec.size(), 11);

    //And again!
    _testData e2;
    value = packer->storeEntry(e);

    ASSERT_EQ(packer->mDataVec.size(), 12);
}
