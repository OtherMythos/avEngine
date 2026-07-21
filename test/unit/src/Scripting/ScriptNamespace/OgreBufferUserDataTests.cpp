#include "gtest/gtest.h"

#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/OgreBufferUserData.h"

using Check = AV::OgreBufferUserData::BufferUploadCheck;

//A plausible point vertex: pos(3f) colour(4f) uv0(4f) uv1(2f).
static const Ogre::uint32 BPE = 13 * 4;
static const size_t NUM_ELEMENTS = 10;

static Check runCheck(size_t blobSizeBytes, SQInteger elementStart, size_t* outCount,
                      Ogre::BufferType type = Ogre::BT_DEFAULT, Ogre::uint32 bpe = BPE){
    return AV::OgreBufferUserData::checkBufferUpload(NUM_ELEMENTS, bpe, blobSizeBytes,
                                                     elementStart, type, outCount);
}

TEST(OgreBufferUserDataTests, uploadFillingWholeBufferSucceeds){
    size_t count = 0;
    EXPECT_EQ(runCheck(NUM_ELEMENTS * BPE, 0, &count), Check::Success);
    EXPECT_EQ(count, NUM_ELEMENTS);
}

TEST(OgreBufferUserDataTests, uploadOfSubRangeSucceeds){
    size_t count = 0;
    EXPECT_EQ(runCheck(3 * BPE, 4, &count), Check::Success);
    EXPECT_EQ(count, 3u);
}

TEST(OgreBufferUserDataTests, uploadEndingExactlyOnLastElementSucceeds){
    //Boundary case - start + count == numElements is in bounds, not off the end.
    size_t count = 0;
    EXPECT_EQ(runCheck(2 * BPE, 8, &count), Check::Success);
    EXPECT_EQ(count, 2u);
}

TEST(OgreBufferUserDataTests, uploadOfSingleElementAtLastIndexSucceeds){
    size_t count = 0;
    EXPECT_EQ(runCheck(BPE, NUM_ELEMENTS - 1, &count), Check::Success);
    EXPECT_EQ(count, 1u);
}

TEST(OgreBufferUserDataTests, blobNotAWholeNumberOfElementsIsRejected){
    size_t count = 1234;
    EXPECT_EQ(runCheck(BPE + 1, 0, &count), Check::MisalignedBlob);
    EXPECT_EQ(count, 0u);
}

TEST(OgreBufferUserDataTests, emptyBlobIsRejected){
    size_t count = 1234;
    EXPECT_EQ(runCheck(0, 0, &count), Check::EmptyBlob);
    EXPECT_EQ(count, 0u);
}

TEST(OgreBufferUserDataTests, negativeStartIsRejected){
    size_t count = 1234;
    EXPECT_EQ(runCheck(BPE, -1, &count), Check::NegativeStart);
    EXPECT_EQ(count, 0u);
}

TEST(OgreBufferUserDataTests, startAtEndOfBufferLeavesNoRoom){
    size_t count = 1234;
    EXPECT_EQ(runCheck(BPE, NUM_ELEMENTS, &count), Check::OutOfBounds);
    EXPECT_EQ(count, 0u);
}

TEST(OgreBufferUserDataTests, startPastEndOfBufferIsRejected){
    size_t count = 0;
    EXPECT_EQ(runCheck(BPE, NUM_ELEMENTS + 5, &count), Check::OutOfBounds);
}

TEST(OgreBufferUserDataTests, overrunningByOneElementIsRejected){
    size_t count = 1234;
    EXPECT_EQ(runCheck((NUM_ELEMENTS + 1) * BPE, 0, &count), Check::OutOfBounds);
    EXPECT_EQ(count, 0u);

    //Same overrun, but reached by starting partway in.
    EXPECT_EQ(runCheck(3 * BPE, NUM_ELEMENTS - 2, &count), Check::OutOfBounds);
}

TEST(OgreBufferUserDataTests, immutableBufferIsRejected){
    //Index buffers are created BT_IMMUTABLE, so this is the case that stops a
    //script uploading to one. Checked before anything else, so an otherwise
    //perfectly valid upload still fails.
    size_t count = 1234;
    EXPECT_EQ(runCheck(NUM_ELEMENTS * BPE, 0, &count, Ogre::BT_IMMUTABLE), Check::ImmutableBuffer);
    EXPECT_EQ(count, 0u);
}

TEST(OgreBufferUserDataTests, zeroBytesPerElementDoesNotDivideByZero){
    size_t count = 1234;
    EXPECT_EQ(runCheck(BPE, 0, &count, Ogre::BT_DEFAULT, 0), Check::MisalignedBlob);
    EXPECT_EQ(count, 0u);
}
