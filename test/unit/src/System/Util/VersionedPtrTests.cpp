#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "System/Util/VersionedPtr.h"

class VersionedPtrTests : public ::testing::Test {
private:
    AV::VersionedPtr<void*> data;
public:
    VersionedPtrTests() {
    }

    virtual ~VersionedPtrTests() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
};

TEST_F(VersionedPtrTests, storePointers){
    void* dataFirst = reinterpret_cast<void*>(100);

    AV::uint64 firstId = data.storeEntry(dataFirst);
    ASSERT_EQ(data._existing.size(), 1);
    ASSERT_EQ(data._pool.getVersions().size(), 1);

    AV::uint64 secondId = data.storeEntry(dataFirst);
    ASSERT_EQ(data._existing.size(), 1);
    ASSERT_EQ(data._pool.getVersions().size(), 1);
    //Should return the same id as it's already stored.
    ASSERT_EQ(firstId, secondId);
    ASSERT_EQ(data._pool.getEntry(firstId).count, 2);
    ASSERT_TRUE(data.isIdValid(firstId));

    ASSERT_FALSE(data.removeEntry(firstId));
    //Nothing should be deleted, however the count should decrease.
    ASSERT_EQ(data._existing.size(), 1);
    ASSERT_EQ(data._pool.getVersions().size(), 1);
    ASSERT_EQ(data._pool.getEntry(firstId).count, 1);
    ASSERT_TRUE(data.isIdValid(firstId));

    ASSERT_TRUE(data.removeEntry(firstId));
    //With the final deletion, the object should become invalid.
    ASSERT_FALSE(data.isIdValid(firstId));

    firstId = data.storeEntry(dataFirst);
    ASSERT_EQ(data._existing.size(), 1);
    ASSERT_EQ(data._pool.getVersions().size(), 1);
    ASSERT_TRUE(data.isIdValid(firstId));
    ASSERT_FALSE(data.isIdValid(secondId));
    ASSERT_NE(firstId, secondId);

    //Remove via pointer.
    ASSERT_TRUE(data.removeEntry(dataFirst));
    ASSERT_FALSE(data.isIdValid(firstId));
}

TEST_F(VersionedPtrTests, storeMultiplePointers){

    static const size_t NUM = 50;
    AV::uint64 ids[NUM];
    for(size_t i = 0; i < NUM; i++){
        void* determinedId = reinterpret_cast<void*>(100 + i);

        ids[i] = data.storeEntry(determinedId);
        ASSERT_EQ(data._existing.size(), i+1);
        ASSERT_EQ(data._pool.getVersions().size(), i+1);
    }

    for(size_t i = 0; i < NUM; i++){
        void* determinedId = reinterpret_cast<void*>(100 + i);
        ASSERT_TRUE(data.doesPtrExist(determinedId));
        ASSERT_TRUE(data.isIdValid(ids[i]));
    }

    void* targetId = reinterpret_cast<void*>(100 + 25);
    ASSERT_TRUE(data.removeEntry(targetId));
    ASSERT_FALSE(data.doesPtrExist(targetId));
    ASSERT_FALSE(data.isIdValid(ids[25]));

    //All the others should exist.
    for(size_t i = 0; i < NUM; i++){
        void* determinedId = reinterpret_cast<void*>(100 + i);

        if(i == 25){
            ASSERT_FALSE(data.doesPtrExist(determinedId));
            ASSERT_FALSE(data.isIdValid(ids[i]));
        }else{
            ASSERT_TRUE(data.doesPtrExist(determinedId));
            ASSERT_TRUE(data.isIdValid(ids[i]));
        }
    }
}

TEST_F(VersionedPtrTests, invalidatePtr){
    void* ptrValue = reinterpret_cast<void*>(125);

    AV::uint64 storedValue = data.storeEntry(ptrValue);
    ASSERT_TRUE(data.isIdValid(storedValue));

    ASSERT_TRUE(data.invalidateEntry(ptrValue));
    ASSERT_FALSE(data.isIdValid(storedValue));
    //The pointer should not still exist.
    ASSERT_FALSE(data.doesPtrExist(ptrValue));
    ASSERT_FALSE(data.removeEntry(ptrValue));
    //From here shouldn't be able to invalidate any further.
    ASSERT_FALSE(data.invalidateEntry(ptrValue));
}
