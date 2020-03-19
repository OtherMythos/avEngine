#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <bitset>

#define private public

#include "Input/InputManager.h"

class InputManagerTests : public ::testing::Test {
private:
    AV::InputManager inMan;
public:
    InputManagerTests() {
    }

    virtual ~InputManagerTests() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
};

TEST_F(InputManagerTests, getActionSetHandleTest){
    //TODO the checked values will need to change at some point. I'd like to wait until I have functions to set these values though.

    //One that exists.
    AV::ActionSetHandle handle = inMan.getActionSetHandle("default");
    ASSERT_EQ(handle, 0);

    //One that doesnt
    handle = inMan.getActionSetHandle("valueThatDoesntExist");
    ASSERT_EQ(handle, AV::INVALID_ACTION_SET_HANDLE);
}

TEST_F(InputManagerTests, getDigitalActionHandleTest){
    AV::ActionHandle handle = inMan.getDigitalActionHandle("Move");

    AV::ActionHandle expectedHandle = inMan._produceActionHandle({AV::InputManager::ActionType::Button, 0, 0});

    // std::bitset<32> x(handle);
    // std::cout << x << '\n';
    // std::bitset<32> y(expectedHandle);
    // std::cout << y << '\n';
    ASSERT_EQ(handle, expectedHandle);
}

TEST_F(InputManagerTests, produceActionHandleTest){
    AV::InputManager::ActionHandleContents contents = {AV::InputManager::ActionType::AnalogTrigger, 255, 255};
    AV::ActionHandle handle = inMan._produceActionHandle(contents);

    //ASSERT_EQ(handle, 0);

    std::bitset<32> x(handle);
    std::cout << x << '\n';


}

TEST_F(InputManagerTests, readActionHandleTest){
    AV::InputManager::ActionHandleContents contents[5] = {
        {AV::InputManager::ActionType::AnalogTrigger, 255, 255},
        {AV::InputManager::ActionType::Button, 0, 0},
        {AV::InputManager::ActionType::StickPadGyro, 10, 20},
        {AV::InputManager::ActionType::AnalogTrigger, 150, 20},
        {AV::InputManager::ActionType::Button, 250, 200},
    };
    for(int i = 0; i < 5; i++){
        AV::ActionHandle handle = inMan._produceActionHandle(contents[i]);

        AV::InputManager::ActionHandleContents outContents;
        inMan._readActionHandle(&outContents, handle);

        ASSERT_EQ(outContents.type, contents[i].type);
        ASSERT_EQ(outContents.actionSetId, contents[i].actionSetId);
        ASSERT_EQ(outContents.itemIdx, contents[i].itemIdx);
    }
}

TEST_F(InputManagerTests, setAndGetButtonAction){
    AV::InputManager::ActionHandleContents contents = {AV::InputManager::ActionType::Button, 0, 0};
    AV::ActionHandle handle = inMan._produceActionHandle(contents);

    bool result = inMan.getDigitalAction(handle);
    ASSERT_FALSE(result);
}
