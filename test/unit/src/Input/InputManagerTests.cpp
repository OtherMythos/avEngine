#include "gtest/gtest.h"
#include "gmock/gmock.h"

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
