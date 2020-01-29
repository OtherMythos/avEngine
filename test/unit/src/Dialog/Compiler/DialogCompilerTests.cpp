#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "Dialog/Compiler/DialogCompiler.h"

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
