#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "Dialog/Compiler/DialogCompiler.h"
#include "Dialog/Compiler/DialogScriptData.h"

class DialogCompilerTests : public ::testing::Test {
private:
    AV::DialogCompiler comp;
public:
    DialogCompilerTests() {
    }

    virtual ~DialogCompilerTests() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
};

TEST_F(DialogCompilerTests, scanStringFindsNothingInEmptyString){
    int result = comp._scanStringForVariables("");

    ASSERT_EQ(result, 0);
}

TEST_F(DialogCompilerTests, scanStringFindsNothingInStringWithNoVariables){
    int result = comp._scanStringForVariables("This is a string with no variables.");

    ASSERT_EQ(result, 0);
}

TEST_F(DialogCompilerTests, scanStringFindsNVariables){
    int result = comp._scanStringForVariables("This is a string $first$ $second$ $third$ $fourth$");
    ASSERT_EQ(result, 4);

    result = comp._scanStringForVariables("This is a string #first# #second# #third# #fourth#");
    ASSERT_EQ(result, 4);

    result = comp._scanStringForVariables("#first#!!");
    ASSERT_EQ(result, 1);

    result = comp._scanStringForVariables("$first$!!");
    ASSERT_EQ(result, 1);

    result = comp._scanStringForVariables("$first$!! #second#");
    ASSERT_EQ(result, 2);
}

TEST_F(DialogCompilerTests, scanStringFindsMalformedVariable){
    int result = comp._scanStringForVariables("$brokenVariable");
    ASSERT_EQ(result, -1);

    result = comp._scanStringForVariables("#brokenVariable");
    ASSERT_EQ(result, -1);

    result = comp._scanStringForVariables("$broken#Vari#able");
    ASSERT_EQ(result, -1);

    result = comp._scanStringForVariables("$broken#Vari#able$");
    ASSERT_EQ(result, -1);

    result = comp._scanStringForVariables("$brokenVariable$#");
    ASSERT_EQ(result, -1);

    result = comp._scanStringForVariables("#");
    ASSERT_EQ(result, -1);

    result = comp._scanStringForVariables("$");
    ASSERT_EQ(result, -1);
}

TEST_F(DialogCompilerTests, attributeOutputToCharTests){
    {
        AV::AttributeOutput o;
        o.globalVariable = false;
        o.isVariable = false;
        char c = comp._attributeOutputToChar(o, AV::AttributeType::INT);

        ASSERT_EQ(c, 0x0 | ((char)AV::AttributeType::INT << 2));
    }

    {
        AV::AttributeOutput o;
        o.globalVariable = false;
        o.isVariable = true;
        char c = comp._attributeOutputToChar(o, AV::AttributeType::FLOAT);

        ASSERT_EQ(c, 0x1 | ((char)AV::AttributeType::FLOAT << 2));
    }

    {
        AV::AttributeOutput o;
        o.globalVariable = true;
        o.isVariable = true;
        char c = comp._attributeOutputToChar(o, AV::AttributeType::BOOLEAN);

        ASSERT_EQ(c, 0x3 | ((char)AV::AttributeType::BOOLEAN << 2));
    }
}

TEST_F(DialogCompilerTests, optionTagInvalidTypes){

    {
        const char* script =
    "        <Dialog_Script>"
    "            <b id = '0'>"
    "                <option>"
    "                    <ts id='1'>Some text.</ts>"
    "                </option>"
    "            </b>"
    "        </Dialog_Script>"
        ;

        AV::CompiledDialog data;
        bool success = comp.compileScript(script, data);
        ASSERT_TRUE(success);
    }

    {
        //Invalid id.
        const char* script =
    "        <Dialog_Script>"
    "            <b id = '0'>"
    "                <option>"
    "                    <ts id='something invalid'>Some text.</ts>"
    "                </option>"
    "            </b>"
    "        </Dialog_Script>"
        ;

        AV::CompiledDialog data;
        bool success = comp.compileScript(script, data);
        ASSERT_FALSE(success);
    }

    {
        //Missing id.
        const char* script =
    "        <Dialog_Script>"
    "            <b id = '0'>"
    "                <option>"
    "                    <ts>Some text.</ts>"
    "                </option>"
    "            </b>"
    "        </Dialog_Script>"
        ;

        AV::CompiledDialog data;
        bool success = comp.compileScript(script, data);
        ASSERT_FALSE(success);
    }

    {
        //Invalid types for id
        const char* script =
    "        <Dialog_Script>"
    "            <b id = '0'>"
    "                <option>"
    "                    <ts id='a string'>Some text.</ts>"
    "                </option>"
    "            </b>"
    "        </Dialog_Script>"
        ;

        AV::CompiledDialog data;
        bool success = comp.compileScript(script, data);
        ASSERT_FALSE(success);
    }
}

TEST_F(DialogCompilerTests, optionTagCheckData){
    {
        const char* script =
    "        <Dialog_Script>"
    "            <b id = '0'>"
    "                <option>"
    "                    <ts id='1'>Some text.</ts>"
    "                </option>"
    "            </b>"
    "        </Dialog_Script>"
        ;

        AV::CompiledDialog data;
        bool success = comp.compileScript(script, data);
        ASSERT_TRUE(success);

        ASSERT_EQ(data.vEntry4List->size(), 1);
        ASSERT_EQ(data.vEntry2List->size(), 1);
        ASSERT_EQ(data.stringList->size(), 1);
    }

    {
        const char* script =
    "        <Dialog_Script>"
    "            <b id = '0'>"
    "                <option>"
    "                    <ts id='1'>first</ts>"
    "                    <ts id='1'>second</ts>"
    "                    <ts id='1'>third</ts>"
    "                    <ts id='1'>fourth</ts>"
    "                    <ts id='1'>Some text.</ts>"
    "                    <ts id='1'>Some text.</ts>"
    "                </option>"
    "            </b>"
    "        </Dialog_Script>"
        ;

        AV::CompiledDialog data;
        bool success = comp.compileScript(script, data);
        ASSERT_TRUE(success);

        //Should never get higher than the max dialog options.
        ASSERT_EQ(data.vEntry4List->size(), 1);
        ASSERT_EQ(data.vEntry2List->size(), AV::MAX_DIALOG_OPTIONS);
        ASSERT_EQ(data.stringList->size(), AV::MAX_DIALOG_OPTIONS);

        ASSERT_TRUE((*data.stringList)[0] == "first");
        ASSERT_TRUE((*data.stringList)[1] == "second");
        ASSERT_TRUE((*data.stringList)[2] == "third");
        ASSERT_TRUE((*data.stringList)[3] == "fourth");
    }
}
