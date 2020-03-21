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

TEST_F(InputManagerTests, getButtonActionHandleTest){
    AV::ActionHandle handle = inMan.getButtonActionHandle("Move");

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

    //std::bitset<32> x(handle);
    //std::cout << x << '\n';


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

//Some of these tests will fail while I sort this out.
TEST_F(InputManagerTests, DISABLED_setAndGetButtonAction){
    AV::InputManager::ActionHandleContents contents = {AV::InputManager::ActionType::Button, 0, 0};
    AV::ActionHandle handle = inMan._produceActionHandle(contents);

    bool result = inMan.getButtonAction(0, handle);
    ASSERT_FALSE(result);

    inMan.setButtonAction(0, handle, true);
    result = inMan.getButtonAction(0, handle);
    ASSERT_TRUE(result);
}

TEST_F(InputManagerTests, setActionSets){
    /*
    Tests the procedure which would be used to create a group of actions and action sets.

    The planned action sets looks like this:
    FirstSet{
        StickPadGyro{
            "Move"
            "Camera"
            "Dodge"
        }
        AnalogTrigger{
            "TriggerAction"
        }
        Buttons{
            "Jump"
            "Attack"
        }
    }
    SecondSet{
        StickPadGyro{
            "MenuMove"
        }
        AnalogTrigger{
            "MenuSkip"
        }
        Buttons{
            "Select"
            "Back"
        }
    }
    */

    inMan.clearAllActionSets();
    AV::ActionSetHandle firstHandle = inMan.createActionSet("FirstSet");
    AV::ActionSetHandle secondHandle = inMan.createActionSet("SecondSet");

    inMan.createAction("Move", firstHandle, AV::InputManager::ActionType::StickPadGyro, true);
    inMan.createAction("Camera", firstHandle, AV::InputManager::ActionType::StickPadGyro, false);
    inMan.createAction("TriggerAction", firstHandle, AV::InputManager::ActionType::AnalogTrigger, true);
    inMan.createAction("Jump", firstHandle, AV::InputManager::ActionType::Button, true);
    inMan.createAction("Attack", firstHandle, AV::InputManager::ActionType::Button, false);
    inMan.createAction("Dodge", firstHandle, AV::InputManager::ActionType::Button, false);

    inMan.createAction("MenuMove", secondHandle, AV::InputManager::ActionType::StickPadGyro, true);
    inMan.createAction("MenuSkip", secondHandle, AV::InputManager::ActionType::AnalogTrigger, true);
    inMan.createAction("Select", secondHandle, AV::InputManager::ActionType::Button, true);
    inMan.createAction("Back", secondHandle, AV::InputManager::ActionType::Button, false);

    //All entries appear in the same lists.
    ASSERT_EQ(inMan.mActionButtonData.size(), 5);
    ASSERT_EQ(inMan.mActionAnalogTriggerData.size(), 2);
    ASSERT_EQ(inMan.mActionStickPadGyroData.size(), 3);

    ASSERT_EQ(inMan.mActionSets.size(), 2); //We've created two action sets.

    ASSERT_EQ(inMan.mActionSets[0].buttonStart, 0);
    ASSERT_EQ(inMan.mActionSets[0].buttonEnd, 3);
    ASSERT_EQ(inMan.mActionSets[0].analogTriggerStart, 0);
    ASSERT_EQ(inMan.mActionSets[0].analogTriggerEnd, 1);
    ASSERT_EQ(inMan.mActionSets[0].stickStart, 0);
    ASSERT_EQ(inMan.mActionSets[0].stickEnd, 2);

    ASSERT_EQ(inMan.mActionSets[1].buttonStart, 3);
    ASSERT_EQ(inMan.mActionSets[1].buttonEnd, 5);
    ASSERT_EQ(inMan.mActionSets[1].analogTriggerStart, 1);
    ASSERT_EQ(inMan.mActionSets[1].analogTriggerEnd, 2);
    ASSERT_EQ(inMan.mActionSets[1].stickStart, 2);
    ASSERT_EQ(inMan.mActionSets[1].stickEnd, 3);

    //The names in the list should be in the order they were submitted.
    const char* expectedEntries[10] = {
        "Move", "Camera", "TriggerAction", "Jump", "Attack", "Dodge",
        "MenuMove", "MenuSkip", "Select", "Back"
    };
    for(int i = 0; i < inMan.mActionSetData.size(); i++){
        const AV::InputManager::ActionSetDataEntry& e = inMan.mActionSetData[i];

        ASSERT_EQ(e.first, expectedEntries[i]);
    }

    ASSERT_EQ(inMan.mActionSetData[0].second, 0);
    ASSERT_EQ(inMan.mActionSetData[1].second, 1);
    ASSERT_EQ(inMan.mActionSetData[2].second, 0);
    ASSERT_EQ(inMan.mActionSetData[3].second, 0);
    ASSERT_EQ(inMan.mActionSetData[4].second, 1);
    ASSERT_EQ(inMan.mActionSetData[5].second, 2);

    ASSERT_EQ(inMan.mActionSetData[6].second, 2);
    ASSERT_EQ(inMan.mActionSetData[7].second, 1);
    ASSERT_EQ(inMan.mActionSetData[8].second, 3);
    ASSERT_EQ(inMan.mActionSetData[9].second, 4);
}
