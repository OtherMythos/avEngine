#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "World/Physics/Worlds/CollisionWorldUtils.h"

#include <cassert>
#include <bitset>
#include <iostream>

TEST(CollisionWorldUtils, producePackedInt){

    {
        int returnVal = AV::CollisionWorldUtils::producePackedInt(
            AV::CollisionObjectType::CollisionObjectType::SENDER_SCRIPT, AV::CollisionObjectTypeMask::PLAYER, AV::CollisionObjectEventMask::LEAVE
        );
        std::bitset<32> bits(returnVal);
        std::bitset<32> bitsSecond("00000000000000010000000100000001");
        ASSERT_EQ(bits, bitsSecond);
    }

    {
        int returnVal = AV::CollisionWorldUtils::producePackedInt(
            AV::CollisionObjectType::CollisionObjectType::RECEIVER,
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
            AV::CollisionObjectType::CollisionObjectType::RECEIVER, AV::CollisionObjectTypeMask::PLAYER,
            AV::CollisionObjectEventMask::LEAVE | AV::CollisionObjectEventMask::ENTER | AV::CollisionObjectEventMask::INSIDE
        );
        std::bitset<32> bits(returnVal);
        std::bitset<32> bitsSecond("00000000000001110000000100000000");
        ASSERT_EQ(bits, bitsSecond);
    }

    //assert(false);
}

TEST(CollisionWorldUtils, readPackedInt){
    {
        int returnVal = AV::CollisionWorldUtils::producePackedInt(AV::CollisionObjectType::CollisionObjectType::SENDER_SCRIPT, AV::CollisionObjectTypeMask::PLAYER, AV::CollisionObjectEventMask::LEAVE);

        AV::CollisionWorldUtils::PackedIntContents contents;
        AV::CollisionWorldUtils::readPackedInt(returnVal, &contents);

        ASSERT_EQ(contents.type, AV::CollisionObjectType::CollisionObjectType::SENDER_SCRIPT);
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
        int returnVal = AV::CollisionWorldUtils::producePackedInt(AV::CollisionObjectType::CollisionObjectType::RECEIVER, objectType, AV::CollisionObjectEventMask::LEAVE);

        AV::CollisionWorldUtils::PackedIntContents contents;
        AV::CollisionWorldUtils::readPackedInt(returnVal, &contents);

        ASSERT_EQ(contents.type, AV::CollisionObjectType::CollisionObjectType::RECEIVER);
        ASSERT_EQ(contents.target, objectType);
    }

    {
        char eventType = AV::CollisionObjectEventMask::LEAVE | AV::CollisionObjectEventMask::ENTER | AV::CollisionObjectEventMask::INSIDE;
        int returnVal = AV::CollisionWorldUtils::producePackedInt(AV::CollisionObjectType::CollisionObjectType::RECEIVER, AV::CollisionObjectTypeMask::PLAYER, eventType);

        AV::CollisionWorldUtils::PackedIntContents contents;
        AV::CollisionWorldUtils::readPackedInt(returnVal, &contents);

        ASSERT_EQ(contents.type, AV::CollisionObjectType::CollisionObjectType::RECEIVER);
        ASSERT_EQ(contents.target, AV::CollisionObjectTypeMask::PLAYER);
        ASSERT_EQ(contents.eventType, eventType);
    }
}
