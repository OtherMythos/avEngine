#ifdef DEBUG_SERVER

#include "gtest/gtest.h"

#include "System/DebugServer/Eval/ScriptEvaluator.h"
#include "../../Scripting/ScriptTestHelper.h"

using namespace AV;

namespace{
    rapidjson::Document evalCode(const std::string& code){
        rapidjson::Document doc;
        ScriptEvaluator::eval(ScriptTestHelper::getVM(), code, doc);
        return doc;
    }
}

TEST(ScriptEvaluatorTests, bareExpressionReturnsValue){
    rapidjson::Document doc = evalCode("1 + 1");
    ASSERT_TRUE(doc["ok"].GetBool());
    ASSERT_EQ(doc["result"].GetInt64(), 2);
}

TEST(ScriptEvaluatorTests, statementsWithExplicitReturn){
    rapidjson::Document doc = evalCode("local total = 0; for(local i = 1; i <= 4; i++) total += i; return total;");
    ASSERT_TRUE(doc["ok"].GetBool());
    ASSERT_EQ(doc["result"].GetInt64(), 10);
}

TEST(ScriptEvaluatorTests, tableSerialisesToJsonObject){
    rapidjson::Document doc = evalCode("return {name = \"player\", hp = 30, tags = [\"a\", \"b\"]};");
    ASSERT_TRUE(doc["ok"].GetBool());
    const rapidjson::Value& result = doc["result"];
    ASSERT_TRUE(result.IsObject());
    ASSERT_STREQ(result["name"].GetString(), "player");
    ASSERT_EQ(result["hp"].GetInt64(), 30);
    ASSERT_TRUE(result["tags"].IsArray());
    ASSERT_EQ(result["tags"].Size(), 2u);
}

TEST(ScriptEvaluatorTests, compileErrorReported){
    rapidjson::Document doc = evalCode("local = broken(");
    ASSERT_FALSE(doc["ok"].GetBool());
    ASSERT_NE(std::string(doc["error"].GetString()).find("compile error"), std::string::npos);
}

TEST(ScriptEvaluatorTests, runtimeErrorReported){
    rapidjson::Document doc = evalCode("thisFunctionDoesNotExist()");
    ASSERT_FALSE(doc["ok"].GetBool());
    ASSERT_NE(std::string(doc["error"].GetString()).find("runtime error"), std::string::npos);
}

TEST(ScriptEvaluatorTests, printsCaptured){
    rapidjson::Document doc = evalCode("print(\"hello agent\"); return 5;");
    ASSERT_TRUE(doc["ok"].GetBool());
    ASSERT_EQ(doc["prints"].Size(), 1u);
    ASSERT_STREQ(doc["prints"][0].GetString(), "hello agent");
}

TEST(ScriptEvaluatorTests, stackRestoredAcrossEvals){
    HSQUIRRELVM vm = ScriptTestHelper::getVM();
    const SQInteger top = sq_gettop(vm);
    evalCode("return [1, 2, 3];");
    evalCode("not even close to valid ((");
    evalCode("thisFunctionDoesNotExist()");
    ASSERT_EQ(sq_gettop(vm), top);
}

#endif
