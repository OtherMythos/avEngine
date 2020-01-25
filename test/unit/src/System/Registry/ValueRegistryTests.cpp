#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "System/Registry/ValueRegistry.h"

class ValueRegistryTests : public ::testing::Test {
private:
    AV::ValueRegistry reg;
public:
    ValueRegistryTests() {
    }

    virtual ~ValueRegistryTests() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
};

TEST_F(ValueRegistryTests, storeFloat){
    const float targetFloat = 10.123f;
    reg.setFloatValue("someName", targetFloat);

    float outVal;
    ASSERT_TRUE(reg.getFloatValue("someName", outVal));

    ASSERT_EQ(outVal, targetFloat);
}

TEST_F(ValueRegistryTests, storeInt){
    const int targetInt = 20;
    reg.setIntValue("someName", targetInt);

    int outVal;
    ASSERT_TRUE(lookupSuccess(reg.getIntValue("someName", outVal)));

    ASSERT_EQ(outVal, targetInt);
}

TEST_F(ValueRegistryTests, storeBool){
    const bool targetBool = false;
    reg.setBoolValue("someName", targetBool);

    bool outVal;
    ASSERT_TRUE(lookupSuccess(reg.getBoolValue("someName", outVal)));

    ASSERT_EQ(outVal, targetBool);
}

TEST_F(ValueRegistryTests, storeString){
    const std::string targetString = "wow a string";
    reg.setStringValue("someName", targetString);

    std::string outVal;
    ASSERT_TRUE(lookupSuccess(reg.getStringValue("someName", outVal)));

    ASSERT_EQ(outVal, targetString);
}

TEST_F(ValueRegistryTests, missingFloatReturnsFailure){
    const float targetVal = 10.0f;

    float i = targetVal;
    ASSERT_FALSE(lookupSuccess(reg.getFloatValue("someName", i)));
    ASSERT_EQ(i, targetVal);
}

TEST_F(ValueRegistryTests, missingIntReturnsFailure){
    const int targetVal = 10;

    int i = targetVal;
    AV::RegistryLookup result = reg.getIntValue("someName", i);

    ASSERT_FALSE(lookupSuccess(result));
    ASSERT_EQ(result, AV::REGISTRY_MISSING);
    ASSERT_EQ(i, targetVal);
}

TEST_F(ValueRegistryTests, missingBoolReturnsFailure){
    const bool targetVal = false;

    bool i = targetVal;
    AV::RegistryLookup result = reg.getBoolValue("someName", i);

    ASSERT_FALSE(lookupSuccess(result));
    ASSERT_EQ(result, AV::REGISTRY_MISSING);
    ASSERT_EQ(i, targetVal);
}

TEST_F(ValueRegistryTests, missingStringReturnsFailure){
    const std::string targetVal = "aString";

    std::string i = targetVal;
    AV::RegistryLookup result = reg.getStringValue("someName", i);

    ASSERT_FALSE(lookupSuccess(result));
    ASSERT_EQ(result, AV::REGISTRY_MISSING);
    ASSERT_EQ(i, targetVal);
}

TEST_F(ValueRegistryTests, intTypeMismatch){
    reg.setIntValue("someName", 10);

    int i;
    AV::RegistryLookup result = reg.getIntValue("someName", i);
    ASSERT_TRUE(lookupSuccess(result));

    float f;
    result = reg.getFloatValue("someName", f);
    ASSERT_EQ(result, AV::REGISTRY_MISMATCH);

    bool b;
    result = reg.getBoolValue("someName", b);
    ASSERT_EQ(result, AV::REGISTRY_MISMATCH);
}

TEST_F(ValueRegistryTests, floatTypeMismatch){
    reg.setFloatValue("someName", 10.0f);

    float f;
    AV::RegistryLookup result = reg.getFloatValue("someName", f);
    ASSERT_TRUE(lookupSuccess(result));

    int i;
    result = reg.getIntValue("someName", i);
    ASSERT_EQ(result, AV::REGISTRY_MISMATCH);

    bool b;
    result = reg.getBoolValue("someName", b);
    ASSERT_EQ(result, AV::REGISTRY_MISMATCH);
}

TEST_F(ValueRegistryTests, boolTypeMismatch){
    reg.setBoolValue("someName", false);

    bool b;
    AV::RegistryLookup result = reg.getBoolValue("someName", b);
    ASSERT_TRUE(lookupSuccess(result));

    int i;
    result = reg.getIntValue("someName", i);
    ASSERT_EQ(result, AV::REGISTRY_MISMATCH);

    float f;
    result = reg.getFloatValue("someName", f);
    ASSERT_EQ(result, AV::REGISTRY_MISMATCH);
}

TEST_F(ValueRegistryTests, overrideValues){
    reg.setBoolValue("someName", false);

    bool b;
    AV::RegistryLookup result = reg.getBoolValue("someName", b);
    ASSERT_TRUE(lookupSuccess(result));
    ASSERT_EQ(b, false);

    reg.setBoolValue("someName", true);
    reg.getBoolValue("someName", b);
    ASSERT_EQ(b, true);

    //Different type now
    reg.setIntValue("someName", 100);
    int i;
    result = reg.getIntValue("someName", i);
    ASSERT_TRUE(lookupSuccess(result));
    ASSERT_EQ(i, 100);

    reg.setFloatValue("someName", 200.0f);
    float f;
    result = reg.getFloatValue("someName", f);
    ASSERT_TRUE(lookupSuccess(result));
    ASSERT_EQ(f, 200.0f);
}

TEST_F(ValueRegistryTests, overrideString){
    reg.setStringValue("someName", "first");
    reg.setStringValue("someName", "second");
    reg.setStringValue("someName", "third");
    reg.setStringValue("someName", "fourth");
    reg.setStringValue("someName", "fifth");

    std::string value;
    reg.getStringValue("someName", value);
    ASSERT_EQ(value, "fifth");

    //The strings vector shouldn't grow if strings are just being replaced.
    ASSERT_EQ(reg.mStrings.size(), 1);
}

TEST_F(ValueRegistryTests, stringOverride){
    reg.setStringValue("someName", "first");
    reg.setBoolValue("someName", false);

    std::string retVal;
    AV::RegistryLookup result = reg.getStringValue("someName", retVal);
    //Should fail, as now it should contain a float.
    ASSERT_FALSE(lookupSuccess(result));

    reg.setStringValue("someName", "second");
    result = reg.getStringValue("someName", retVal);
    ASSERT_EQ(retVal, "second");
    ASSERT_EQ(reg.mStrings.size(), 1);
}
