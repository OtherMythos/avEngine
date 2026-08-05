#include "gtest/gtest.h"

#include "Scripting/SquirrelDeepCopy.h"

#include <squirrel.h>
#include <sqstdaux.h>
#include <string>

using namespace AV;

namespace {
    //Two bare vms, no engine and no ScriptVM, since the copier deliberately depends on neither.
    class DeepCopyFixture : public ::testing::Test{
    protected:
        void SetUp() override{
            src = sq_open(1024);
            dst = sq_open(1024);
        }
        void TearDown() override{
            sq_close(src);
            sq_close(dst);
        }

        //Compile and run an expression, leaving its value on top of vm.
        void pushExpression(HSQUIRRELVM vm, const std::string& expression){
            const std::string source = "return (" + expression + ");";
            ASSERT_TRUE(SQ_SUCCEEDED(sq_compilebuffer(vm, source.c_str(), source.size(), "test", SQTrue)));
            sq_pushroottable(vm);
            ASSERT_TRUE(SQ_SUCCEEDED(sq_call(vm, 1, SQTrue, SQTrue)));
            //Remove the closure from underneath the returned value.
            sq_remove(vm, -2);
        }

        HSQUIRRELVM src = 0;
        HSQUIRRELVM dst = 0;
    };

    //Fetch a string keyed member of the table at idx and return its integer value.
    SQInteger getIntMember(HSQUIRRELVM vm, SQInteger idx, const char* key){
        const SQInteger absIdx = idx < 0 ? sq_gettop(vm) + idx + 1 : idx;
        sq_pushstring(vm, key, -1);
        EXPECT_TRUE(SQ_SUCCEEDED(sq_get(vm, absIdx)));
        SQInteger out = 0;
        sq_getinteger(vm, -1, &out);
        sq_pop(vm, 1);
        return out;
    }
}

TEST_F(DeepCopyFixture, copiesEveryPrimitiveType){
    struct Case{ const char* expression; SQObjectType type; };
    const Case cases[] = {
        {"null", OT_NULL},
        {"true", OT_BOOL},
        {"42", OT_INTEGER},
        {"1.5", OT_FLOAT},
        {"\"hello\"", OT_STRING},
    };

    for(const Case& c : cases){
        pushExpression(src, c.expression);

        SquirrelDeepCopy copy;
        ASSERT_TRUE(copy.deepCopyValue(src, dst, -1)) << c.expression << ": " << copy.getError();
        ASSERT_FALSE(copy.hasError());
        ASSERT_EQ(sq_gettype(dst, -1), c.type) << c.expression;

        sq_pop(src, 1);
        sq_pop(dst, 1);
    }
}

TEST_F(DeepCopyFixture, copiedStringHasItsOwnStorageInTheDestinationVm){
    pushExpression(src, "\"a string long enough not to be interned oddly\"");

    SquirrelDeepCopy copy;
    ASSERT_TRUE(copy.deepCopyValue(src, dst, -1));

    HSQOBJECT srcObj, dstObj;
    sq_resetobject(&srcObj);
    sq_resetobject(&dstObj);
    sq_getstackobj(src, -1, &srcObj);
    sq_getstackobj(dst, -1, &dstObj);

    //Two vms have separate string tables. If these matched, the object would be shared rather
    //than copied, which is exactly the situation this class exists to prevent.
    ASSERT_NE(srcObj._unVal.raw, dstObj._unVal.raw);

    const SQChar* srcStr = 0;
    const SQChar* dstStr = 0;
    sq_getstring(src, -1, &srcStr);
    sq_getstring(dst, -1, &dstStr);
    ASSERT_STREQ(srcStr, dstStr);
}

TEST_F(DeepCopyFixture, copiesNestedTablesAndArraysPreservingKeyTypes){
    pushExpression(src, "{ name = \"outer\", nested = { value = 7 }, list = [1, 2, [3, 4]], [10] = 100 }");

    SquirrelDeepCopy copy;
    ASSERT_TRUE(copy.deepCopyValue(src, dst, -1)) << copy.getError();
    ASSERT_EQ(sq_gettype(dst, -1), OT_TABLE);

    //A string key, an integer key, a nested table and a nested array all have to survive.
    sq_pushstring(dst, "name", -1);
    ASSERT_TRUE(SQ_SUCCEEDED(sq_get(dst, -2)));
    const SQChar* name = 0;
    sq_getstring(dst, -1, &name);
    ASSERT_STREQ(name, "outer");
    sq_pop(dst, 1);

    sq_pushinteger(dst, 10);
    ASSERT_TRUE(SQ_SUCCEEDED(sq_get(dst, -2)));
    SQInteger intKeyed = 0;
    sq_getinteger(dst, -1, &intKeyed);
    ASSERT_EQ(intKeyed, 100);
    sq_pop(dst, 1);

    sq_pushstring(dst, "nested", -1);
    ASSERT_TRUE(SQ_SUCCEEDED(sq_get(dst, -2)));
    ASSERT_EQ(sq_gettype(dst, -1), OT_TABLE);
    ASSERT_EQ(getIntMember(dst, -1, "value"), 7);
    sq_pop(dst, 1);

    sq_pushstring(dst, "list", -1);
    ASSERT_TRUE(SQ_SUCCEEDED(sq_get(dst, -2)));
    ASSERT_EQ(sq_gettype(dst, -1), OT_ARRAY);
    ASSERT_EQ(sq_getsize(dst, -1), 3);
    sq_pushinteger(dst, 2);
    ASSERT_TRUE(SQ_SUCCEEDED(sq_get(dst, -2)));
    ASSERT_EQ(sq_gettype(dst, -1), OT_ARRAY);
    ASSERT_EQ(sq_getsize(dst, -1), 2);
    sq_pop(dst, 2);
}

TEST_F(DeepCopyFixture, copiedContainerIsIndependentOfTheOriginal){
    pushExpression(src, "{ value = 1 }");

    SquirrelDeepCopy copy;
    ASSERT_TRUE(copy.deepCopyValue(src, dst, -1));

    //Mutate the source. The copy must not see it.
    sq_pushstring(src, "value", -1);
    sq_pushinteger(src, 99);
    ASSERT_TRUE(SQ_SUCCEEDED(sq_set(src, -3)));

    ASSERT_EQ(getIntMember(dst, -1, "value"), 1);
}

TEST_F(DeepCopyFixture, aDagIsDuplicatedRatherThanRejected){
    //The same subtable referenced from two places is not a cycle. Value semantics mean it is
    //copied twice, and the two copies must be independent.
    pushExpression(src, "(function(){ local shared = { value = 1 }; return { a = shared, b = shared }; })()");

    SquirrelDeepCopy copy;
    ASSERT_TRUE(copy.deepCopyValue(src, dst, -1)) << copy.getError();
    ASSERT_FALSE(copy.hasError());

    sq_pushstring(dst, "a", -1);
    ASSERT_TRUE(SQ_SUCCEEDED(sq_get(dst, -2)));
    sq_pushstring(dst, "value", -1);
    sq_pushinteger(dst, 50);
    ASSERT_TRUE(SQ_SUCCEEDED(sq_set(dst, -3)));
    sq_pop(dst, 1);

    sq_pushstring(dst, "b", -1);
    ASSERT_TRUE(SQ_SUCCEEDED(sq_get(dst, -2)));
    ASSERT_EQ(getIntMember(dst, -1, "value"), 1);
    sq_pop(dst, 1);
}

TEST_F(DeepCopyFixture, rejectsTypesWhichCannotCrossBetweenVms){
    //These used to be sq_move'd, which splices an object owned by one vm's shared state into the
    //other's reference graph.
    struct Case{ const char* expression; const char* typeNameInError; };
    const Case cases[] = {
        {"function(){}", "closure"},
        {"class{}", "class"},
        {"(class{ constructor(){} })()", "instance"},
        {"print", "native closure"},
    };

    for(const Case& c : cases){
        pushExpression(src, c.expression);

        const SQInteger srcTop = sq_gettop(src);
        const SQInteger dstTop = sq_gettop(dst);

        SquirrelDeepCopy copy;
        ASSERT_FALSE(copy.deepCopyValue(src, dst, -1)) << c.expression;
        ASSERT_TRUE(copy.hasError()) << c.expression;
        ASSERT_NE(copy.getError().find(c.typeNameInError), std::string::npos)
            << c.expression << " produced: " << copy.getError();

        //Nothing pushed, nothing left behind.
        ASSERT_EQ(sq_gettop(src), srcTop);
        ASSERT_EQ(sq_gettop(dst), dstTop);

        sq_pop(src, 1);
    }
}

TEST_F(DeepCopyFixture, rejectsAnUncopyableValueNestedInsideAContainer){
    pushExpression(src, "{ ok = 1, segments = [ { onPaint = function(){} } ] }");

    SquirrelDeepCopy copy;
    ASSERT_FALSE(copy.deepCopyValue(src, dst, -1));
    ASSERT_TRUE(copy.hasError());

    //The message has to say where it was, not just that something failed.
    const std::string& error = copy.getError();
    ASSERT_NE(error.find("closure"), std::string::npos) << error;
    ASSERT_NE(error.find("segments[0].onPaint"), std::string::npos) << error;
}

TEST_F(DeepCopyFixture, pushNullPolicyCopiesLossilyInsteadOfFailing){
    pushExpression(src, "{ value = 1, fn = function(){} }");

    SquirrelDeepCopy copy;
    copy.mUnsupportedPolicy = SquirrelDeepCopy::UnsupportedPolicy::PushNull;
    ASSERT_TRUE(copy.deepCopyValue(src, dst, -1)) << copy.getError();
    ASSERT_FALSE(copy.hasError());

    ASSERT_EQ(getIntMember(dst, -1, "value"), 1);

    sq_pushstring(dst, "fn", -1);
    ASSERT_TRUE(SQ_SUCCEEDED(sq_get(dst, -2)));
    ASSERT_EQ(sq_gettype(dst, -1), OT_NULL);
    sq_pop(dst, 1);
}

TEST_F(DeepCopyFixture, rejectsASelfReferencingTableWithoutHanging){
    pushExpression(src, "(function(){ local t = {}; t.self <- t; return t; })()");

    SquirrelDeepCopy copy;
    ASSERT_FALSE(copy.deepCopyValue(src, dst, -1));
    ASSERT_NE(copy.getError().find("cyclic"), std::string::npos) << copy.getError();
}

TEST_F(DeepCopyFixture, rejectsAMutualCycleBetweenTwoTables){
    pushExpression(src, "(function(){ local a = {}; local b = {}; a.b <- b; b.a <- a; return a; })()");

    SquirrelDeepCopy copy;
    ASSERT_FALSE(copy.deepCopyValue(src, dst, -1));
    ASSERT_NE(copy.getError().find("cyclic"), std::string::npos) << copy.getError();
}

TEST_F(DeepCopyFixture, rejectsACycleThroughAnArray){
    pushExpression(src, "(function(){ local a = []; a.append(a); return a; })()");

    SquirrelDeepCopy copy;
    ASSERT_FALSE(copy.deepCopyValue(src, dst, -1));
    ASSERT_NE(copy.getError().find("cyclic"), std::string::npos) << copy.getError();
}

TEST_F(DeepCopyFixture, rejectsAValueNestedDeeperThanTheDepthLimit){
    pushExpression(src,
        "(function(){"
        "  local root = {};"
        "  local cur = root;"
        "  for(local i = 0; i < 100; i++){ local next = {}; cur.child <- next; cur = next; }"
        "  return root;"
        "})()");

    SquirrelDeepCopy copy;
    copy.mMaxDepth = 64;
    ASSERT_FALSE(copy.deepCopyValue(src, dst, -1));
    ASSERT_NE(copy.getError().find("depth"), std::string::npos) << copy.getError();
}

TEST_F(DeepCopyFixture, stackIsBalancedOnSuccessAndUntouchedOnFailure){
    //The contract the whole class rests on: src unchanged either way, dst +1 on success and
    //unchanged on failure. Several of the old error paths popped the wrong number of values.
    struct Case{ const char* expression; bool shouldSucceed; };
    const Case cases[] = {
        {"1", true},
        {"{ a = 1, b = [1, 2, 3] }", true},
        {"[ { x = 1 }, { y = 2 } ]", true},
        {"function(){}", false},
        {"{ a = 1, fn = function(){} }", false},
        {"[ 1, 2, class{} ]", false},
    };

    for(const Case& c : cases){
        pushExpression(src, c.expression);

        const SQInteger srcTop = sq_gettop(src);
        const SQInteger dstTop = sq_gettop(dst);

        SquirrelDeepCopy copy;
        const bool result = copy.deepCopyValue(src, dst, -1);
        ASSERT_EQ(result, c.shouldSucceed) << c.expression << ": " << copy.getError();

        ASSERT_EQ(sq_gettop(src), srcTop) << c.expression;
        ASSERT_EQ(sq_gettop(dst), dstTop + (c.shouldSucceed ? 1 : 0)) << c.expression;

        sq_settop(src, srcTop - 1);
        sq_settop(dst, dstTop);
    }
}

TEST_F(DeepCopyFixture, negativeIndicesAreResolvedAgainstTheSourceTop){
    //deepCopyTable is called with a negative index by out of tree callers, and it pushes onto the
    //source stack while iterating, so the index has to be resolved up front.
    pushExpression(src, "{ value = 5 }");
    sq_pushinteger(src, 1234); //Something above the table, so -1 is not the table.

    SquirrelDeepCopy copy;
    ASSERT_TRUE(copy.deepCopyTable(src, dst, -2)) << copy.getError();
    ASSERT_EQ(sq_gettype(dst, -1), OT_TABLE);
    ASSERT_EQ(getIntMember(dst, -1, "value"), 5);
}

TEST_F(DeepCopyFixture, copyValueStrictReportsTheReasonItFailed){
    pushExpression(src, "function(){}");

    std::string error;
    ASSERT_FALSE(SquirrelDeepCopy::copyValueStrict(src, dst, -1, error));
    ASSERT_FALSE(error.empty());
    ASSERT_NE(error.find("closure"), std::string::npos) << error;

    sq_pop(src, 1);
    pushExpression(src, "{ ok = 1 }");
    ASSERT_TRUE(SquirrelDeepCopy::copyValueStrict(src, dst, -1, error));
}
