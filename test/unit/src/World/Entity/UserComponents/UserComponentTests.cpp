#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "World/Entity/UserComponents/UserComponentManager.h"
#include "System/SystemSetup/SystemSettings.h"

class UserComponentTests : public ::testing::Test {

    protected:
    std::unique_ptr<AV::UserComponentManager> manager;

    UserComponentTests() {
    }

    virtual ~UserComponentTests() {
    }

    virtual void SetUp() {
        manager = std::unique_ptr<AV::UserComponentManager>(new AV::UserComponentManager());

        AV::SystemSettings::mUserComponentSettings.vars[0] = {
            "testComp0",
            AV::_dataTypesToCombination({AV::ComponentDataTypes::INT, AV::ComponentDataTypes::NONE, AV::ComponentDataTypes::NONE, AV::ComponentDataTypes::NONE}), 1
        };
        AV::SystemSettings::mUserComponentSettings.vars[1] = {
            "testComp1",
            AV::_dataTypesToCombination({AV::ComponentDataTypes::INT, AV::ComponentDataTypes::FLOAT, AV::ComponentDataTypes::NONE, AV::ComponentDataTypes::NONE}), 2
        };
        AV::SystemSettings::mUserComponentSettings.vars[2] = {
            "testComp2",
            AV::_dataTypesToCombination({AV::ComponentDataTypes::INT, AV::ComponentDataTypes::FLOAT, AV::ComponentDataTypes::BOOL, AV::ComponentDataTypes::NONE}), 3
        };
        AV::SystemSettings::mUserComponentSettings.vars[3] = {
            "testComp3",
            AV::_dataTypesToCombination({AV::ComponentDataTypes::INT, AV::ComponentDataTypes::FLOAT, AV::ComponentDataTypes::BOOL, AV::ComponentDataTypes::INT}), 4
        };
    }

    virtual void TearDown() {
        //memset(&(AV::SystemSettings::mUserComponentSettings), 0, sizeof(AV::UserComponentSettings));
    }
};

TEST_F(UserComponentTests, createComponentOfType){
    for(int i = 0; i < 5; i++){
        AV::ComponentId id = manager->createComponentOfType(0);
        ASSERT_EQ(id, i);
        id = manager->createComponentOfType(1);
        ASSERT_EQ(id, i);
        id = manager->createComponentOfType(2);
        ASSERT_EQ(id, i);
        id = manager->createComponentOfType(3);
        ASSERT_EQ(id, i);
    }

    for(int i = 0; i < 4; i++){
        //Should fill the gap
        manager->removeComponent(0, i);
        AV::ComponentId id = manager->createComponentOfType(i);
        ASSERT_EQ(id, 0);
        //Creating a new one should be pushed to the end.
        id = manager->createComponentOfType(i);
        ASSERT_EQ(id, 5);
    }

}

TEST_F(UserComponentTests, setGetComponentValues){
    /*AV::SystemSettings::mUserComponentSettings.vars[0] = {
        "testComp0",
        AV::_dataTypesToCombination({AV::ComponentDataTypes::INT}), 1
    };
    AV::SystemSettings::mUserComponentSettings.vars[1] = {
        "testComp1",
        AV::_dataTypesToCombination({AV::ComponentDataTypes::INT}), 1
    };
    AV::SystemSettings::mUserComponentSettings.vars[2] = {
        "testComp2",
        AV::_dataTypesToCombination({AV::ComponentDataTypes::INT}), 1
    };
    AV::SystemSettings::mUserComponentSettings.vars[3] = {
        "testComp3",
        AV::_dataTypesToCombination({AV::ComponentDataTypes::INT}), 1
    };*/

    for(int i = 0; i < 5; i++){
        AV::ComponentId id = manager->createComponentOfType(0);
        ASSERT_EQ(id, i);
        id = manager->createComponentOfType(1);
        ASSERT_EQ(id, i);
        id = manager->createComponentOfType(2);
        ASSERT_EQ(id, i);
        id = manager->createComponentOfType(3);
        ASSERT_EQ(id, i);
    }

    for(int i = 0; i < 4; i++){
        AV::UserComponentDataEntry val;
        val.i = 10*i;
        manager->setValue(i, 0, i, val);
    }
    for(int i = 0; i < 4; i++){
        AV::UserComponentDataEntry val;
        val.i = 10*i;
        AV::UserComponentDataEntry retVal = manager->getValue(i, 0, i);
        ASSERT_EQ(retVal.i, val.i);
    }
}

TEST_F(UserComponentTests, setGetComponentValuesSingleVar){
    AV::SystemSettings::mUserComponentSettings.vars[0] = {
        "testComp0",
        AV::_dataTypesToCombination({AV::ComponentDataTypes::INT}), 1
    };
    AV::SystemSettings::mUserComponentSettings.vars[1] = {
        "testComp1",
        AV::_dataTypesToCombination({AV::ComponentDataTypes::INT}), 1
    };
    AV::SystemSettings::mUserComponentSettings.vars[2] = {
        "testComp2",
        AV::_dataTypesToCombination({AV::ComponentDataTypes::INT}), 1
    };
    AV::SystemSettings::mUserComponentSettings.vars[3] = {
        "testComp3",
        AV::_dataTypesToCombination({AV::ComponentDataTypes::INT}), 1
    };

    for(int i = 0; i < 5; i++){
        AV::ComponentId id = manager->createComponentOfType(0);
        id = manager->createComponentOfType(1);
        id = manager->createComponentOfType(2);
        id = manager->createComponentOfType(3);
    }

    for(int i = 0; i < 4; i++){
        AV::UserComponentDataEntry val;
        val.i = 10*i;
        manager->setValue(i, 0, 0, val);
    }
    for(int i = 0; i < 4; i++){
        AV::UserComponentDataEntry val;
        val.i = 10*i;
        AV::UserComponentDataEntry retVal = manager->getValue(i, 0, 0);
        ASSERT_EQ(retVal.i, val.i);
    }
}
