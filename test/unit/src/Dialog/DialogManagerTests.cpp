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
    /*std::string str = man._produceDialogVariableString("This is a string $First$", "var");
    ASSERT_EQ(str, "This is a string var");

    str = man._produceDialogVariableString("This is a string $First$$Second$#Third##Fourth#", "var");
    ASSERT_EQ(str, "This is a string varvarvarvar");

    str = man._produceDialogVariableString("This is a string #Something#", "var");
    ASSERT_EQ(str, "This is a string var");

    str = man._produceDialogVariableString("#Something#", "var");
    ASSERT_EQ(str, "var");*/
}

TEST_F(DialogManagerTests, readVariableChar){

    {
        AV::DialogManager::VariableCharContents out;
        char c = 0x0 | ((char)AV::AttributeType::INT << 2);
        man._readVariableChar(c, out);
        ASSERT_FALSE(out.isVariable);
        ASSERT_FALSE(out.isGlobal);
        ASSERT_EQ(AV::AttributeType::INT, out.type);
    }

    {
        AV::DialogManager::VariableCharContents out;
        char c = 0x1 | ((char)AV::AttributeType::FLOAT << 2);
        man._readVariableChar(c, out);
        ASSERT_TRUE(out.isVariable);
        ASSERT_FALSE(out.isGlobal);
        ASSERT_EQ(AV::AttributeType::FLOAT, out.type);
    }

    {
        AV::DialogManager::VariableCharContents out;
        char c = 0x3 | ((char)AV::AttributeType::BOOLEAN << 2);
        man._readVariableChar(c, out);
        ASSERT_TRUE(out.isVariable);
        ASSERT_TRUE(out.isGlobal);
        ASSERT_EQ(AV::AttributeType::BOOLEAN, out.type);
    }
}
