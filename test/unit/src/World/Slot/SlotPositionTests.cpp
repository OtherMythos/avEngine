#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "World/WorldSingleton.h"
#include "World/Slot/SlotPosition.h"
#include "System/SystemSetup/SystemSettings.h"

TEST(SlotPositionTests, SlotPositionDefaultConstructor){
    AV::SlotPosition pos;

    ASSERT_EQ(0, pos.chunkX());
    ASSERT_EQ(0, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 0, 0), pos.position());
}

TEST(SlotPositionTests, SlotPositionSetValuesConstructor){
    AV::SlotPosition pos(1, 2, Ogre::Vector3(3, 4, 5));

    ASSERT_EQ(1, pos.chunkX());
    ASSERT_EQ(2, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(3, 4, 5), pos.position());
}

TEST(SlotPositionTests, SlotPositionConstructorClamping){
    AV::SystemSettings::_worldSlotSize = 100;

    AV::SlotPosition pos(1, 2, Ogre::Vector3(-1, 200, 100));
    ASSERT_EQ(Ogre::Vector3(0, 200, 100), pos.position());

    pos = AV::SlotPosition(1, 2, Ogre::Vector3(200, 0, -200));
    ASSERT_EQ(Ogre::Vector3(100, 0, 0), pos.position());


    AV::SystemSettings::_worldSlotSize = 1000;
    pos = AV::SlotPosition(1, 2, Ogre::Vector3(200, 0, 0));
    ASSERT_EQ(Ogre::Vector3(200, 0, 0), pos.position());

    pos = AV::SlotPosition(1, 2, Ogre::Vector3(2000, 0, 0));
    ASSERT_EQ(Ogre::Vector3(1000, 0, 0), pos.position());
}

TEST(SlotPositionTests, SlotPositionCopyConstructor){
    AV::SlotPosition pos(1, 2, Ogre::Vector3(3, 4, 5));

    ASSERT_EQ(1, pos.chunkX());
    ASSERT_EQ(2, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(3, 4, 5), pos.position());

    //
    pos = AV::SlotPosition(-10, -20, Ogre::Vector3(50, 50, 50));

    ASSERT_EQ(-10, pos.chunkX());
    ASSERT_EQ(-20, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(50, 50, 50), pos.position());

    //
    AV::SystemSettings::_worldSlotSize = 100;
    pos = AV::SlotPosition(0, 0, Ogre::Vector3(-10, -10, 300));

    ASSERT_EQ(0, pos.chunkX());
    ASSERT_EQ(0, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, -10, 100), pos.position());
}

TEST(SlotPositionTests, SlotPositionConstructorOgreVector){
    AV::WorldSingleton::_origin = AV::SlotPosition();
    AV::SystemSettings::_worldSlotSize = 100;

    AV::SlotPosition pos(Ogre::Vector3(0, 0, 0));
    ASSERT_EQ(0, pos.chunkX());
    ASSERT_EQ(0, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 0, 0), pos.position());

    //
    pos = AV::SlotPosition(Ogre::Vector3(99, 10000, 99));
    ASSERT_EQ(0, pos.chunkX());
    ASSERT_EQ(0, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(99, 10000, 99), pos.position());

    //
    pos = AV::SlotPosition(Ogre::Vector3(200, 0, -200));
    ASSERT_EQ(2, pos.chunkX());
    ASSERT_EQ(-2, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 0, 0), pos.position());

    //
    pos = AV::SlotPosition(Ogre::Vector3(250, 0, -250));
    ASSERT_EQ(2, pos.chunkX());
    ASSERT_EQ(-3, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(50, 0, 50), pos.position());

    //
    pos = AV::SlotPosition(Ogre::Vector3(-500, 0, 10000));
    ASSERT_EQ(-5, pos.chunkX());
    ASSERT_EQ(100, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 0, 0), pos.position());
}

TEST(SlotPositionTests, SlotPositionConstructorOgreVectorOriginShift){
    //Origin 100, 100
    AV::WorldSingleton::_origin = AV::SlotPosition(1, 1);
    AV::SystemSettings::_worldSlotSize = 100;

    AV::SlotPosition pos(Ogre::Vector3(0, 0, 0));
    ASSERT_EQ(1, pos.chunkX());
    ASSERT_EQ(1, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 0, 0), pos.position());

    //
    pos = AV::SlotPosition(Ogre::Vector3(-500, 0, -500));
    ASSERT_EQ(-4, pos.chunkX());
    ASSERT_EQ(-4, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 0, 0), pos.position());

    //
    AV::WorldSingleton::_origin = AV::SlotPosition(10, 10, Ogre::Vector3(50, 0, 50));
    pos = AV::SlotPosition(Ogre::Vector3(-1, 0, -1));
    ASSERT_EQ(10, pos.chunkX());
    ASSERT_EQ(10, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(49, 0, 49), pos.position());

    //
    pos = AV::SlotPosition(Ogre::Vector3(-51, 0, -51));
    ASSERT_EQ(9, pos.chunkX());
    ASSERT_EQ(9, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(99, 0, 99), pos.position());

    //
    pos = AV::SlotPosition(Ogre::Vector3(525, 0, -525));
    ASSERT_EQ(15, pos.chunkX());
    ASSERT_EQ(5, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(75, 0, 25), pos.position());

    //
    AV::WorldSingleton::_origin = AV::SlotPosition(-10, 10, Ogre::Vector3(75, 0, 25));
    pos = AV::SlotPosition(Ogre::Vector3(525, 0, -525));
    ASSERT_EQ(-4, pos.chunkX());
    ASSERT_EQ(5, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 0, 0), pos.position());

    //
    AV::WorldSingleton::_origin = AV::SlotPosition(-10, 10, Ogre::Vector3(75, 0, 25));
    pos = AV::SlotPosition(Ogre::Vector3(525, 0, -526));
    ASSERT_EQ(-4, pos.chunkX());
    ASSERT_EQ(4, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 0, 99), pos.position());
}

TEST(SlotPositionTests, SlotPositionAssignmentOperator){
    AV::SlotPosition pos(1, 2, Ogre::Vector3(3, 4, 5));

    pos = AV::SlotPosition(0, 0, Ogre::Vector3(0, 0, 0));

    ASSERT_EQ(0, pos.chunkX());
    ASSERT_EQ(0, pos.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 0, 0), pos.position());
}

TEST(SlotPositionTests, SlotPositionEqualsOperator){
    AV::SlotPosition pos(1, 2, Ogre::Vector3(3, 4, 5));
    AV::SlotPosition pos2(100, 200, Ogre::Vector3(50, 50, 50));

    ASSERT_TRUE(pos == AV::SlotPosition(1, 2, Ogre::Vector3(3, 4, 5)));
    ASSERT_TRUE(pos2 == AV::SlotPosition(100, 200, Ogre::Vector3(50, 50, 50)));
}

TEST(SlotPositionTests, SlotPositionAdditionOperatorSlots){
    AV::SlotPosition pos1(1, 2, Ogre::Vector3(3, 4, 5));
    AV::SlotPosition pos2(3, 4, Ogre::Vector3(3, 4, 5));

    AV::SlotPosition result = pos1 + pos2;

    ASSERT_EQ(4, result.chunkX());
    ASSERT_EQ(6, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(6, 8, 10), result.position());

    pos1 = AV::SlotPosition(-100, -100, Ogre::Vector3(3, 4, 5));
    pos2 = AV::SlotPosition(100, 100, Ogre::Vector3(3, 4, 5));

    result = pos1 + pos2;

    ASSERT_EQ(0, result.chunkX());
    ASSERT_EQ(0, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(6, 8, 10), result.position());
}

TEST(SlotPositionTests, SlotPositionAdditionOperatorPositionOverflow){
    AV::SystemSettings::_worldSlotSize = 100;

    AV::SlotPosition pos1(0, 0, Ogre::Vector3(0, 0, 50));
    AV::SlotPosition pos2(0, 0, Ogre::Vector3(0, 0, 50));

    AV::SlotPosition result = pos1 + pos2;

    //No overflow
    ASSERT_EQ(0, result.chunkX());
    ASSERT_EQ(0, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 0, 100), result.position());

    //Overflow by 1
    pos1 = AV::SlotPosition(0, 0, Ogre::Vector3(0, 0, 51));
    pos2 = AV::SlotPosition(0, 0, Ogre::Vector3(0, 0, 50));

    result = pos1 + pos2;

    ASSERT_EQ(0, result.chunkX());
    ASSERT_EQ(1, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 0, 1), result.position());

    //Random numbers
    pos1 = AV::SlotPosition(3, 4, Ogre::Vector3(50, 0, 20));
    pos2 = AV::SlotPosition(1, 7, Ogre::Vector3(90, 0, 57));

    result = pos1 + pos2;
    ASSERT_EQ(5, result.chunkX());
    ASSERT_EQ(11, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(40, 0, 77), result.position());
}

TEST(SlotPositionTests, SlotPositionAdditionOperatorPositionNegativeOverflow){
    AV::SystemSettings::_worldSlotSize = 100;

    AV::SlotPosition pos1(-100, 25, Ogre::Vector3(0, 0, 50));
    AV::SlotPosition pos2(-100, 25, Ogre::Vector3(0, 0, 50));

    AV::SlotPosition result = pos1 + pos2;

    //No overflow
    ASSERT_EQ(-200, result.chunkX());
    ASSERT_EQ(50, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 0, 100), result.position());

    //Overflow by 1
    pos1 = AV::SlotPosition(-50, -30, Ogre::Vector3(0, 0, 51));
    pos2 = AV::SlotPosition(-25, -20, Ogre::Vector3(0, 0, 50));

    result = pos1 + pos2;
    ASSERT_EQ(-75, result.chunkX());
    ASSERT_EQ(-49, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 0, 1), result.position());

    //
    pos1 = AV::SlotPosition(-500, 31, Ogre::Vector3(100, 0, 1));
    pos2 = AV::SlotPosition(500, -23, Ogre::Vector3(100, 0, 5));

    result = pos1 + pos2;
    ASSERT_EQ(1, result.chunkX());
    ASSERT_EQ(8, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(100, 0, 6), result.position());
}

TEST(SlotPositionTests, SlotPositionMinusOperator){
    AV::SystemSettings::_worldSlotSize = 100;

    AV::SlotPosition pos1(5, 6, Ogre::Vector3(0, 0, 0));
    AV::SlotPosition pos2(5, 5, Ogre::Vector3(0, 0, 0));

    AV::SlotPosition result = pos1 - pos2;

    ASSERT_EQ(0, result.chunkX());
    ASSERT_EQ(1, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 0, 0), result.position());

    //Negative overflow
    pos1 = AV::SlotPosition(10, -100, Ogre::Vector3(0, 0, 10));
    pos2 = AV::SlotPosition(1, -10, Ogre::Vector3(0, 0, 90));

    result = pos1 - pos2;
    ASSERT_EQ(9, result.chunkX());
    ASSERT_EQ(-91, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 0, 20), result.position());

    //
    pos1 = AV::SlotPosition(0, 200, Ogre::Vector3(1, 0, 10));
    pos2 = AV::SlotPosition(10, -10, Ogre::Vector3(1, 0, 70));

    result = pos1 - pos2;
    ASSERT_EQ(-10, result.chunkX());
    ASSERT_EQ(209, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 0, 40), result.position());

    //
    pos1 = AV::SlotPosition(-100, 200, Ogre::Vector3(20, 40, 20));
    pos2 = AV::SlotPosition(-100, -10, Ogre::Vector3(77, 85, 70));

    result = pos1 - pos2;
    ASSERT_EQ(-1, result.chunkX());
    ASSERT_EQ(209, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(43, -45, 50), result.position());
}

TEST(SlotPositionTests, SlotPositionPlusOgreOperator){
    AV::SystemSettings::_worldSlotSize = 100;
    AV::SlotPosition pos(0, 0, Ogre::Vector3(0, 0, 0));

    AV::SlotPosition result = pos + Ogre::Vector3(10, 10, 10);

    ASSERT_EQ(0, result.chunkX());
    ASSERT_EQ(0, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(10, 10, 10), result.position());

    //
    pos = AV::SlotPosition(10, 20, Ogre::Vector3(0, 0, 0));

    result = pos + Ogre::Vector3(200, 150, 50);

    ASSERT_EQ(12, result.chunkX());
    ASSERT_EQ(20, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 150, 50), result.position());

    //
    pos = AV::SlotPosition(0, 0, Ogre::Vector3(0, 0, 0));

    result = pos + Ogre::Vector3(2001, 10, -2001);

    ASSERT_EQ(20, result.chunkX());
    ASSERT_EQ(-21, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(1, 10, 99), result.position());

    //
    pos = AV::SlotPosition(11, -12, Ogre::Vector3(50, 10, 60));

    result = pos + Ogre::Vector3(150, 10, -120);

    ASSERT_EQ(13, result.chunkX());
    ASSERT_EQ(-13, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 20, 40), result.position());

    //
    pos = AV::SlotPosition(5, 6, Ogre::Vector3(10, 10, 30));

    result = pos + Ogre::Vector3(-2000, -100, -3001);

    ASSERT_EQ(-15, result.chunkX());
    ASSERT_EQ(-24, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(10, -90, 29), result.position());
}

TEST(SlotPositionTests, SlotPositionMinusOgreOperator){
    AV::SystemSettings::_worldSlotSize = 100;
    AV::SlotPosition pos(0, 0, Ogre::Vector3(0, 0, 0));

    AV::SlotPosition result = pos - Ogre::Vector3(10, 10, 10);

    ASSERT_EQ(-1, result.chunkX());
    ASSERT_EQ(-1, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(90, -10, 90), result.position());

    //
    pos = AV::SlotPosition(0, 0, Ogre::Vector3(0, 0, 0));

    result = pos - Ogre::Vector3(350, 0, 450);

    ASSERT_EQ(-4, result.chunkX());
    ASSERT_EQ(-5, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(50, 0, 50), result.position());

    //
    pos = AV::SlotPosition(10, 20, Ogre::Vector3(0, 0, 0));

    result = pos - Ogre::Vector3(200, 150, 50);

    ASSERT_EQ(8, result.chunkX());
    ASSERT_EQ(19, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, -150, 50), result.position());

    pos = AV::SlotPosition(10, 20, Ogre::Vector3(0, 0, 0));

    //
    pos = AV::SlotPosition(0, 0, Ogre::Vector3(0, 0, 0));

    result = pos - Ogre::Vector3(2001, 10, -2001);

    ASSERT_EQ(-21, result.chunkX());
    ASSERT_EQ(20, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(99, -10, 1), result.position());

    //
    pos = AV::SlotPosition(11, -12, Ogre::Vector3(50, 10, 60));

    result = pos - Ogre::Vector3(150, 10, -120);

    ASSERT_EQ(10, result.chunkX());
    ASSERT_EQ(-11, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(0, 0, 80), result.position());

    //
    pos = AV::SlotPosition(5, 6, Ogre::Vector3(10, 10, 30));

    result = pos - Ogre::Vector3(-2000, -100, -3001);

    ASSERT_EQ(25, result.chunkX());
    ASSERT_EQ(36, result.chunkY());
    ASSERT_EQ(Ogre::Vector3(10, 110, 31), result.position());
}

TEST(SlotPositionTests, SlotPositionToOgre){
    AV::SystemSettings::_worldSlotSize = 100;
    AV::WorldSingleton::_origin = AV::SlotPosition();

    AV::SlotPosition pos(0, 0, Ogre::Vector3(0, 0, 0));

    ASSERT_EQ(Ogre::Vector3(0, 0, 0), pos.toOgre());

    //
    pos = AV::SlotPosition(1, 1, Ogre::Vector3(0, 0, 0));
    ASSERT_EQ(Ogre::Vector3(100, 0, 100), pos.toOgre());

    //
    pos = AV::SlotPosition(-1, -1, Ogre::Vector3(0, 0, 0));
    ASSERT_EQ(Ogre::Vector3(-100, 0, -100), pos.toOgre());

    //
    pos = AV::SlotPosition(-1, -1, Ogre::Vector3(50, 0, 50));
    ASSERT_EQ(Ogre::Vector3(-50, 0, -50), pos.toOgre());

    //
    AV::WorldSingleton::_origin = AV::SlotPosition(-1, -1);

    pos = AV::SlotPosition(0, 0, Ogre::Vector3(0, 0, 0));
    ASSERT_EQ(Ogre::Vector3(100, 0, 100), pos.toOgre());

    //
    AV::WorldSingleton::_origin = AV::SlotPosition(100, 100);

    pos = AV::SlotPosition(99, 99, Ogre::Vector3(0, 0, 0));
    ASSERT_EQ(Ogre::Vector3(-100, 0, -100), pos.toOgre());

    //
    AV::WorldSingleton::_origin = AV::SlotPosition(100, 100, Ogre::Vector3(50, 0, 50));

    pos = AV::SlotPosition(99, 99, Ogre::Vector3(0, 0, 0));
    ASSERT_EQ(Ogre::Vector3(-150, 0, -150), pos.toOgre());

    //
    AV::WorldSingleton::_origin = AV::SlotPosition(30, 30, Ogre::Vector3(50, 0, 50));

    pos = AV::SlotPosition(10, 10, Ogre::Vector3(50, 100, 50));
    ASSERT_EQ(Ogre::Vector3(-2000, 100, -2000), pos.toOgre());
}

TEST(SlotPositionTests, SlotPositionPlusEqualsSlotPosition){
    AV::SystemSettings::_worldSlotSize = 100;
    AV::WorldSingleton::_origin = AV::SlotPosition();

    AV::SlotPosition pos(0, 0, Ogre::Vector3(0, 0, 0));
    AV::SlotPosition second(1, 1);
    pos += second;

    ASSERT_EQ(pos, second);

    //
    pos = AV::SlotPosition(0, 0, Ogre::Vector3(50, 100, 50));
    second = AV::SlotPosition(1, 1);
    pos += second;

    ASSERT_EQ(pos, AV::SlotPosition(1, 1, Ogre::Vector3(50, 100, 50)));

    //
    pos = AV::SlotPosition(0, 0, Ogre::Vector3(50, 100, 50));
    second = AV::SlotPosition(1, 1, Ogre::Vector3(50, 100, 50));
    pos += second;

    ASSERT_EQ(pos, AV::SlotPosition(1, 1, Ogre::Vector3(100, 200, 100)));

    //
    pos = AV::SlotPosition(6, 5, Ogre::Vector3(50, 100, 50));
    second = AV::SlotPosition(3, 4, Ogre::Vector3(90, 100, 90));
    pos += second;

    ASSERT_EQ(pos, AV::SlotPosition(10, 10, Ogre::Vector3(40, 200, 40)));

    //
    pos = AV::SlotPosition(10, 10, Ogre::Vector3(50, 100, 50));
    second = AV::SlotPosition(3, 4, Ogre::Vector3(20, 100, 25));
    pos += second;

    ASSERT_EQ(pos, AV::SlotPosition(13, 14, Ogre::Vector3(70, 200, 75)));

    //
    pos = AV::SlotPosition(-3, -5, Ogre::Vector3(50, 100, 50));
    second = AV::SlotPosition(6, 7, Ogre::Vector3(20, 100, 25));
    pos += second;

    ASSERT_EQ(pos, AV::SlotPosition(3, 2, Ogre::Vector3(70, 200, 75)));

    //
    pos = AV::SlotPosition(-1, -2, Ogre::Vector3(50, 100, 50));
    second = AV::SlotPosition(1, 2, Ogre::Vector3(25, 100, 25));
    pos += second;

    ASSERT_EQ(pos, AV::SlotPosition(0, 0, Ogre::Vector3(75, 200, 75)));
}

TEST(SlotPositionTests, SlotPositionPlusEqualsVec3){
    AV::SystemSettings::_worldSlotSize = 100;
    AV::WorldSingleton::_origin = AV::SlotPosition();

    AV::SlotPosition pos(0, 0, Ogre::Vector3(0, 0, 0));
    AV::SlotPosition second(1, 1);

    pos = AV::SlotPosition(0, 0, Ogre::Vector3(50, 100, 50));
    pos += Ogre::Vector3(25, 100, 25);

    ASSERT_EQ(pos, AV::SlotPosition(0, 0, Ogre::Vector3(75, 200, 75)));

    //Checking the shifted origin doesn't interfere.
    AV::WorldSingleton::_origin = AV::SlotPosition(0, 0, Ogre::Vector3(10, 10, 10));
    pos = AV::SlotPosition(0, 0, Ogre::Vector3(50, 100, 50));
    pos += Ogre::Vector3(25, 100, 25);

    ASSERT_EQ(pos, AV::SlotPosition(0, 0, Ogre::Vector3(75, 200, 75)));
}

TEST(SlotPositionTests, SlotPositionMinusEqualsVec3){
    AV::SystemSettings::_worldSlotSize = 100;
    AV::WorldSingleton::_origin = AV::SlotPosition();

    AV::SlotPosition pos(0, 0, Ogre::Vector3(0, 0, 0));
    AV::SlotPosition second(1, 1);

    pos = AV::SlotPosition(0, 0, Ogre::Vector3(50, 100, 50));
    pos -= Ogre::Vector3(25, 100, 25);

    ASSERT_EQ(pos, AV::SlotPosition(0, 0, Ogre::Vector3(25, 0, 25)));

    //Checking the shifted origin doesn't interfere.
    AV::WorldSingleton::_origin = AV::SlotPosition(0, 0, Ogre::Vector3(10, 10, 10));
    pos = AV::SlotPosition(0, 0, Ogre::Vector3(50, 100, 50));
    pos -= Ogre::Vector3(25, 100, 25);

    ASSERT_EQ(pos, AV::SlotPosition(0, 0, Ogre::Vector3(25, 0, 25)));
}

TEST(SlotPositionTests, SlotPositionMinusEqualsSlotPosition){
    AV::SystemSettings::_worldSlotSize = 100;
    AV::WorldSingleton::_origin = AV::SlotPosition();

    AV::SlotPosition pos(0, 0, Ogre::Vector3(0, 0, 0));
    pos -= AV::SlotPosition(1, 1);

    ASSERT_EQ(pos, AV::SlotPosition(-1, -1));

    //
    pos = AV::SlotPosition(0, 0, Ogre::Vector3(50, 100, 50));
    AV::SlotPosition second = AV::SlotPosition(1, 2, Ogre::Vector3(25, 100, 25));
    pos -= second;

    ASSERT_EQ(pos, AV::SlotPosition(-1, -2, Ogre::Vector3(25, 0, 25)));

    //
    pos = AV::SlotPosition(3, 4, Ogre::Vector3(0, 0, 0));
    second = AV::SlotPosition(1, 2, Ogre::Vector3(0, 0, 0));
    pos -= second;

    ASSERT_EQ(pos, AV::SlotPosition(2, 2, Ogre::Vector3(0, 0, 0)));

    //
    pos = AV::SlotPosition(3, 4, Ogre::Vector3(0, 0, 0));
    second = AV::SlotPosition(1, 2, Ogre::Vector3(50, 0, 50));
    pos -= second;

    ASSERT_EQ(pos, AV::SlotPosition(1, 1, Ogre::Vector3(50, 0, 50)));

    //
    pos = AV::SlotPosition(-2, -4, Ogre::Vector3(0, 0, 0));
    second = AV::SlotPosition(1, 2, Ogre::Vector3(0, 0, 0));
    pos -= second;

    ASSERT_EQ(pos, AV::SlotPosition(-3, -6, Ogre::Vector3(0, 0, 0)));

    //
    pos = AV::SlotPosition(-2, -4, Ogre::Vector3(0, 0, 0));
    second = AV::SlotPosition(1, 2, Ogre::Vector3(50, 0, 25));
    pos -= second;

    ASSERT_EQ(pos, AV::SlotPosition(-4, -7, Ogre::Vector3(50, 0, 75)));

    //
    pos = AV::SlotPosition(-2, -4, Ogre::Vector3(0, 0, 0));
    second = AV::SlotPosition(-1, -2, Ogre::Vector3(0, 0, 0));
    pos -= second;

    ASSERT_EQ(pos, AV::SlotPosition(-1, -2, Ogre::Vector3(0, 0, 0)));

    //
    pos = AV::SlotPosition(-2, -4, Ogre::Vector3(0, 0, 0));
    second = AV::SlotPosition(-1, -2, Ogre::Vector3(25, 0, 75));
    pos -= second;

    ASSERT_EQ(pos, AV::SlotPosition(-2, -3, Ogre::Vector3(75, 0, 25)));
}

TEST(SlotPositionTests, SlotPositionMoveTowards){
    return;
    AV::SystemSettings::_worldSlotSize = 100;
    AV::WorldSingleton::_origin = AV::SlotPosition();

    AV::SlotPosition pos(0, 0, Ogre::Vector3(0, 0, 0));
    pos.moveTowards(AV::SlotPosition(0, 0, Ogre::Vector3(1, 0, 0)), 1.0f);
    ASSERT_EQ(pos, AV::SlotPosition(0, 0, Ogre::Vector3(1, 0, 0)));

    //With a bigger destination.
    pos = AV::SlotPosition(0, 0, Ogre::Vector3(0, 0, 0));
    pos.moveTowards(AV::SlotPosition(0, 0, Ogre::Vector3(100, 0, 0)), 1.0f);
    ASSERT_EQ(pos, AV::SlotPosition(0, 0, Ogre::Vector3(1, 0, 0)));

    //With negative position.
    pos = AV::SlotPosition(0, 0, Ogre::Vector3(0, 0, 0));
    pos.moveTowards(AV::SlotPosition(-1, 0, Ogre::Vector3(50, 0, 0)), 1.0f);
    ASSERT_EQ(pos, AV::SlotPosition(-1, 0, Ogre::Vector3(99, 0, 0)));

    //Negative with a larger step.
    pos = AV::SlotPosition(0, 0, Ogre::Vector3(0, 0, 0));
    pos.moveTowards(AV::SlotPosition(-1, 0, Ogre::Vector3(50, 0, 0)), 30.0f);
    ASSERT_EQ(pos, AV::SlotPosition(-1, 0, Ogre::Vector3(70, 0, 0)));
}

TEST(SlotPositionTests, SlotPositionMoveTowardsOvershoots){
    AV::SystemSettings::_worldSlotSize = 100;
    AV::WorldSingleton::_origin = AV::SlotPosition();

    AV::SlotPosition pos(0, 0, Ogre::Vector3(0, 0, 0));
    pos.moveTowards(AV::SlotPosition(0, 0, Ogre::Vector3(30, 0, 0)), 40.0f);
    ASSERT_EQ(pos, AV::SlotPosition(0, 0, Ogre::Vector3(30, 0, 0)));

    //Should move backwards
    pos = AV::SlotPosition(0, 0, Ogre::Vector3(60, 0, 0));
    pos.moveTowards(AV::SlotPosition(0, 0, Ogre::Vector3(30, 0, 0)), 50.0f);
    ASSERT_EQ(pos, AV::SlotPosition(0, 0, Ogre::Vector3(30, 0, 0)));

    //Shouldn't quite make it.
    pos = AV::SlotPosition(-1, 0, Ogre::Vector3(60, 0, 0));
    pos.moveTowards(AV::SlotPosition(0, 0, Ogre::Vector3(30, 0, 0)), 50.0f);
    ASSERT_EQ(pos, AV::SlotPosition(0, 0, Ogre::Vector3(10, 0, 0)));
}
