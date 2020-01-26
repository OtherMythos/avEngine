#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "Dialog/DialogManager.h"

class DialogManagerTests : public ::testing::Test {
private:
    AV::DialogManager man;
public:
    DialogManagerTests() {
    }

    virtual ~DialogManagerTests() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
};

TEST_F(DialogManagerTests, produceStringWithNoVariables){
    const std::string initString("This is a string");
    const std::string str = man._produceDialogVariableString(initString, "var");

    ASSERT_EQ(str, initString);
}

TEST_F(DialogManagerTests, produceStringVariables){
    std::string str = man._produceDialogVariableString("This is a string $First$", "var");
    ASSERT_EQ(str, "This is a string var");

    str = man._produceDialogVariableString("This is a string $First$$Second$#Third##Fourth#", "var");
    ASSERT_EQ(str, "This is a string varvarvarvar");

    str = man._produceDialogVariableString("This is a string #Something#", "var");
    ASSERT_EQ(str, "This is a string var");

    str = man._produceDialogVariableString("#Something#", "var");
    ASSERT_EQ(str, "var");
}
