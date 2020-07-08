#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "World/Physics/Worlds/CollisionWorldUtils.h"

#include <cassert>
#include <bitset>
#include <iostream>

TEST(CollisionWorldUtils, producePackedInt){

    {
        int returnVal = AV::CollisionWorldUtils::producePackedInt(
            AV::CollisionObjectType::CollisionObjectType::SENDER_SCRIPT, 1, AV::CollisionObjectTypeMask::PLAYER, AV::CollisionObjectEventMask::LEAVE
        );
        std::bitset<32> bits(returnVal);
        std::bitset<32> bitsSecond("00000001000000010000000100000001");
        ASSERT_EQ(bits, bitsSecond);
    }

    {
        int returnVal = AV::CollisionWorldUtils::producePackedInt(
            AV::CollisionObjectType::CollisionObjectType::RECEIVER, 0,
                AV::CollisionObjectTypeMask::PLAYER |
                AV::CollisionObjectTypeMask::ENEMY |
                AV::CollisionObjectTypeMask::OBJECT |
                AV::CollisionObjectTypeMask::USER_3 |
                AV::CollisionObjectTypeMask::USER_4 |
                AV::CollisionObjectTypeMask::USER_5 |
                AV::CollisionObjectTypeMask::USER_6
            , AV::CollisionObjectEventMask::LEAVE
        );
        std::bitset<32> bits(returnVal);
        std::bitset<32> bitsSecond("00000000000000010111111100000000");
        ASSERT_EQ(bits, bitsSecond);
    }

    {
        int returnVal = AV::CollisionWorldUtils::producePackedInt(
            AV::CollisionObjectType::CollisionObjectType::RECEIVER, 3, AV::CollisionObjectTypeMask::PLAYER,
            AV::CollisionObjectEventMask::LEAVE | AV::CollisionObjectEventMask::ENTER | AV::CollisionObjectEventMask::INSIDE
        );
        std::bitset<32> bits(returnVal);
        std::bitset<32> bitsSecond("00000011000001110000000100000000");
        ASSERT_EQ(bits, bitsSecond);
    }

    //assert(false);
}

TEST(CollisionWorldUtils, readPackedInt){
    {
        int returnVal = AV::CollisionWorldUtils::producePackedInt(AV::CollisionObjectType::CollisionObjectType::SENDER_SCRIPT, 1, AV::CollisionObjectTypeMask::PLAYER, AV::CollisionObjectEventMask::LEAVE);

        AV::CollisionWorldUtils::PackedIntContents contents;
        AV::CollisionWorldUtils::readPackedInt(returnVal, &contents);

        ASSERT_EQ(contents.type, AV::CollisionObjectType::CollisionObjectType::SENDER_SCRIPT);
        ASSERT_EQ(contents.worldId, 1);
    }

    {
        char objectType =
        AV::CollisionObjectTypeMask::PLAYER |
        AV::CollisionObjectTypeMask::ENEMY |
        AV::CollisionObjectTypeMask::OBJECT |
        AV::CollisionObjectTypeMask::USER_3 |
        AV::CollisionObjectTypeMask::USER_4 |
        AV::CollisionObjectTypeMask::USER_5 |
        AV::CollisionObjectTypeMask::USER_6;
        int returnVal = AV::CollisionWorldUtils::producePackedInt(AV::CollisionObjectType::CollisionObjectType::RECEIVER, 2, objectType, AV::CollisionObjectEventMask::LEAVE);

        AV::CollisionWorldUtils::PackedIntContents contents;
        AV::CollisionWorldUtils::readPackedInt(returnVal, &contents);

        ASSERT_EQ(contents.type, AV::CollisionObjectType::CollisionObjectType::RECEIVER);
        ASSERT_EQ(contents.target, objectType);
        ASSERT_EQ(contents.worldId, 2);
    }

    {
        char eventType = AV::CollisionObjectEventMask::LEAVE | AV::CollisionObjectEventMask::ENTER | AV::CollisionObjectEventMask::INSIDE;
        int returnVal = AV::CollisionWorldUtils::producePackedInt(AV::CollisionObjectType::CollisionObjectType::RECEIVER, 3, AV::CollisionObjectTypeMask::PLAYER, eventType);

        AV::CollisionWorldUtils::PackedIntContents contents;
        AV::CollisionWorldUtils::readPackedInt(returnVal, &contents);

        ASSERT_EQ(contents.type, AV::CollisionObjectType::CollisionObjectType::RECEIVER);
        ASSERT_EQ(contents.target, AV::CollisionObjectTypeMask::PLAYER);
        ASSERT_EQ(contents.eventType, eventType);
        ASSERT_EQ(contents.worldId, 3);
    }
}

TEST(CollisionWorldUtils, shouldObjectsSendEventTestsReturnsFalseWithSameObjectTypes){
    //Two receiver values, so it should fail.
    int val0 = AV::CollisionWorldUtils::producePackedInt(AV::CollisionObjectType::CollisionObjectType::RECEIVER, 0, AV::CollisionObjectTypeMask::PLAYER, AV::CollisionObjectEventMask::ENTER);
    int val1 = AV::CollisionWorldUtils::producePackedInt(AV::CollisionObjectType::CollisionObjectType::RECEIVER, 0, AV::CollisionObjectTypeMask::PLAYER, AV::CollisionObjectEventMask::ENTER);

    ASSERT_FALSE(AV::CollisionWorldUtils::shouldObjectsSendEvent(AV::CollisionObjectEventMask::CollisionObjectEventMask::LEAVE, val0, val1));
}

TEST(CollisionWorldUtils, shouldObjectsSendEventTestsReturnsTrue){
    { //Check the events masks.
        AV::CollisionObjectEventMask::CollisionObjectEventMask eventMasks[3] = {AV::CollisionObjectEventMask::ENTER, AV::CollisionObjectEventMask::INSIDE, AV::CollisionObjectEventMask::LEAVE};
        for(int i = 0; i < 3; i++){
            int val0 = AV::CollisionWorldUtils::producePackedInt(AV::CollisionObjectType::CollisionObjectType::RECEIVER, 0, AV::CollisionObjectTypeMask::PLAYER, eventMasks[i]);
            int val1 = AV::CollisionWorldUtils::producePackedInt(AV::CollisionObjectType::CollisionObjectType::SENDER_SCRIPT, 0, AV::CollisionObjectTypeMask::PLAYER, eventMasks[i]);

            ASSERT_TRUE(AV::CollisionWorldUtils::shouldObjectsSendEvent(eventMasks[i], val0, val1));
        }
    }

    { //Check the type masks.
        AV::CollisionObjectTypeMask::CollisionObjectTypeMask typeMasks[7] = {
            AV::CollisionObjectTypeMask::PLAYER,
            AV::CollisionObjectTypeMask::ENEMY,
            AV::CollisionObjectTypeMask::OBJECT,
            AV::CollisionObjectTypeMask::USER_3,
            AV::CollisionObjectTypeMask::USER_4,
            AV::CollisionObjectTypeMask::USER_5,
            AV::CollisionObjectTypeMask::USER_6
        };
        for(int i = 0; i < 7; i++){
            int val0 = AV::CollisionWorldUtils::producePackedInt(AV::CollisionObjectType::CollisionObjectType::RECEIVER, 0, typeMasks[i], AV::CollisionObjectEventMask::LEAVE);
            int val1 = AV::CollisionWorldUtils::producePackedInt(AV::CollisionObjectType::CollisionObjectType::SENDER_SCRIPT, 0, typeMasks[i], AV::CollisionObjectEventMask::LEAVE);

            ASSERT_TRUE(AV::CollisionWorldUtils::shouldObjectsSendEvent(AV::CollisionObjectEventMask::LEAVE, val0, val1));
        }
    }
}

TEST(CollisionWorldUtils, shouldObjectsSendEventReturnsFalseWithIncorrectValues){
    //If the type values are different, the event should not be sent.
    typedef std::pair<AV::CollisionObjectTypeMask::CollisionObjectTypeMask, AV::CollisionObjectEventMask::CollisionObjectEventMask> TestData;
    typedef std::pair<TestData, TestData> TestDataEntry;

    static const char dataSize = 4;
    TestDataEntry data[dataSize] = {
        //Correct type, incorrect event type.
        { {AV::CollisionObjectTypeMask::PLAYER, AV::CollisionObjectEventMask::INSIDE}, {AV::CollisionObjectTypeMask::PLAYER, AV::CollisionObjectEventMask::INSIDE} },
        //Incorrect type, correct event type
        { {AV::CollisionObjectTypeMask::ENEMY, AV::CollisionObjectEventMask::LEAVE}, {AV::CollisionObjectTypeMask::PLAYER, AV::CollisionObjectEventMask::LEAVE} },
        //Incorrect multiple types, correct event type
        { {AV::CollisionObjectTypeMask::CollisionObjectTypeMask(AV::CollisionObjectTypeMask::ENEMY | AV::CollisionObjectTypeMask::OBJECT | AV::CollisionObjectTypeMask::USER_3),
        AV::CollisionObjectEventMask::LEAVE}, {AV::CollisionObjectTypeMask::PLAYER, AV::CollisionObjectEventMask::LEAVE} },
        //Both incorrect.
        { {AV::CollisionObjectTypeMask::ENEMY, AV::CollisionObjectEventMask::INSIDE}, {AV::CollisionObjectTypeMask::PLAYER, AV::CollisionObjectEventMask::INSIDE} }
    };

    for(int i = 0; i < dataSize; i++){
        int val0 = AV::CollisionWorldUtils::producePackedInt(AV::CollisionObjectType::CollisionObjectType::SENDER_SCRIPT, 0, data[i].first.first, data[i].first.second);
        int val1 = AV::CollisionWorldUtils::producePackedInt(AV::CollisionObjectType::CollisionObjectType::RECEIVER, 0, data[i].second.first, data[i].second.second);

        ASSERT_FALSE(AV::CollisionWorldUtils::shouldObjectsSendEvent(AV::CollisionObjectEventMask::CollisionObjectEventMask::LEAVE, val0, val1));
    }
}
