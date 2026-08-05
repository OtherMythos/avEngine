#include "gtest/gtest.h"

#include "Scripting/ScriptNamespace/SystemNamespace.h"

#include <squirrel.h>
#include <rapidjson/document.h>
#include <string>

/*
_readJsonValue must push exactly one value for every json number, whatever its
magnitude.

This is the regression guard for a silent corruption: the number branch used to test
only IsInt() and IsDouble(). rapidjson reports both as false for an integer outside
int32 (a uint32 hash, say), so such a value pushed NOTHING - and because the caller
unconditionally does sq_arrayappend / sq_newslot afterwards, the array or table
around it was built from whatever happened to be on the stack instead. Nothing threw;
the parsed data was just quietly wrong.

The tests go through the reader directly rather than through readJSONAsTable so they
touch no filesystem and stay well inside the 30ms budget.
*/

namespace {
    class TestableSystemNamespace : public AV::SystemNamespace{
    public:
        using AV::SystemNamespace::_readJsonValue;
    };

    class JsonReadValueFixture : public ::testing::Test{
    protected:
        void SetUp() override { vm = sq_open(1024); }
        void TearDown() override { sq_close(vm); }

        //Parse a json document whose sole member "v" is the value under test, and push it.
        //Returns how many stack entries the push added, which is the property that actually
        //matters: anything other than exactly 1 corrupts the enclosing container.
        int pushValueOf(const std::string& jsonLiteral){
            rapidjson::Document d;
            d.Parse((std::string("{\"v\":") + jsonLiteral + "}").c_str());
            EXPECT_FALSE(d.HasParseError()) << "test's own json is malformed: " << jsonLiteral;

            const SQInteger before = sq_gettop(vm);
            TestableSystemNamespace::_readJsonValue(vm, d["v"]);
            return static_cast<int>(sq_gettop(vm) - before);
        }

        SQInteger topInteger(){
            SQInteger out = 0;
            EXPECT_EQ(sq_gettype(vm, -1), OT_INTEGER);
            sq_getinteger(vm, -1, &out);
            return out;
        }

        HSQUIRRELVM vm = 0;
    };
}

TEST_F(JsonReadValueFixture, everyNumberKindPushesExactlyOneValue){
    //The uint32 cases are the ones that used to push nothing at all.
    const char* literals[] = {
        "0", "1", "-1",
        "2147483647",           //INT32_MAX
        "-2147483648",          //INT32_MIN
        "2147483648",           //INT32_MAX + 1: not IsInt(), not IsDouble()
        "4294967295",           //UINT32_MAX, as produced by a >>> 0 hash
        "4254967176",           //a real tileHash output
        "9223372036854775807",  //INT64_MAX
        "1.5", "-1.5", "0.0", "1e10",
    };
    for(const char* literal : literals){
        EXPECT_EQ(pushValueOf(literal), 1) << "pushed the wrong number of values for " << literal;
        sq_settop(vm, 0);
    }
}

TEST_F(JsonReadValueFixture, integersBeyondInt32SurviveAsIntegers){
    //SQInteger is 64 bit, so these must arrive exactly - not as floats, and not truncated.
    ASSERT_EQ(pushValueOf("4294967295"), 1);
    ASSERT_EQ(topInteger(), 4294967295LL);
    sq_settop(vm, 0);

    ASSERT_EQ(pushValueOf("4254967176"), 1);
    ASSERT_EQ(topInteger(), 4254967176LL);
    sq_settop(vm, 0);

    ASSERT_EQ(pushValueOf("-4294967295"), 1);
    ASSERT_EQ(topInteger(), -4294967295LL);
}

TEST_F(JsonReadValueFixture, int32RangeStillArrivesAsInteger){
    ASSERT_EQ(pushValueOf("-2147483648"), 1);
    ASSERT_EQ(topInteger(), -2147483648LL);
    sq_settop(vm, 0);

    ASSERT_EQ(pushValueOf("2147483647"), 1);
    ASSERT_EQ(topInteger(), 2147483647LL);
}

TEST_F(JsonReadValueFixture, fractionalNumbersArriveAsFloats){
    ASSERT_EQ(pushValueOf("1.5"), 1);
    ASSERT_EQ(sq_gettype(vm, -1), OT_FLOAT);
    SQFloat f = 0.0f;
    sq_getfloat(vm, -1, &f);
    ASSERT_FLOAT_EQ(f, 1.5f);
}

TEST_F(JsonReadValueFixture, anArrayOfLargeIntegersKeepsEveryElement){
    //The end-to-end shape of the original bug: the elements went missing, so the array
    //came out short and every index past the first bad value was wrong.
    rapidjson::Document d;
    d.Parse("{\"v\":[4294967295, 1, 4254967176, 2, 2147483648]}");
    ASSERT_FALSE(d.HasParseError());

    sq_newarray(vm, 0);
    for(auto itr = d["v"].Begin(); itr != d["v"].End(); ++itr){
        TestableSystemNamespace::_readJsonValue(vm, *itr);
        sq_arrayappend(vm, -2);
    }

    ASSERT_EQ(sq_getsize(vm, -1), 5);

    const SQInteger expected[] = {4294967295LL, 1, 4254967176LL, 2, 2147483648LL};
    for(SQInteger i = 0; i < 5; i++){
        sq_pushinteger(vm, i);
        ASSERT_TRUE(SQ_SUCCEEDED(sq_get(vm, -2))) << "element " << i << " missing";
        ASSERT_EQ(topInteger(), expected[i]) << "element " << i << " wrong";
        sq_pop(vm, 1);
    }
}
