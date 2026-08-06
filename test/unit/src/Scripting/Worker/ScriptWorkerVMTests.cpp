#include "gtest/gtest.h"

#include "Scripting/Worker/ScriptWorkerVM.h"

#include <squirrel.h>
#include <string>
#include <vector>

using namespace AV;

/*
This file is a contract test rather than a behaviour test. The value of a worker vm is entirely in
what is NOT in it: the moment something reaches the scene graph, the entity system or Ogre, a
worker thread can corrupt the engine. The absent list below is the thing worth keeping green.
*/

namespace {
    class WorkerVMFixture : public ::testing::Test{
    protected:
        //A null owner is fine here - the namespaces which need one check for it, and this test
        //only inspects the shape of the root table.
        void SetUp() override { vm = ScriptWorkerVM::create(0); }
        void TearDown() override { sq_close(vm); }

        //Evaluate an expression and return it as a bool.
        bool evalBool(const std::string& expression){
            const std::string source = "return (" + expression + ");";
            if(SQ_FAILED(sq_compilebuffer(vm, source.c_str(), source.size(), "test", SQTrue))){
                ADD_FAILURE() << "could not compile: " << expression;
                return false;
            }
            sq_pushroottable(vm);
            if(SQ_FAILED(sq_call(vm, 1, SQTrue, SQTrue))){
                ADD_FAILURE() << "could not run: " << expression;
                sq_settop(vm, 0);
                return false;
            }

            SQBool result = SQFalse;
            sq_getbool(vm, -1, &result);
            sq_pop(vm, 2);

            return result == SQTrue;
        }

        bool rootHas(const std::string& name){
            return evalBool("\"" + name + "\" in getroottable()");
        }

        HSQUIRRELVM vm = 0;
    };
}

TEST_F(WorkerVMFixture, theRootTableHasNothingWhichReachesTheEngine){
    //Every one of these touches state a worker thread must never see. If a change to
    //ScriptWorkerVM::create makes one of these appear, that change is a bug.
    const std::vector<std::string> forbidden = {
        "_scene", "_entity", "_component", "_gui", "_physics", "_camera", "_mesh",
        "_animation", "_input", "_window", "_audio", "_timer", "_state",
        "_compositor", "_graphics", "_hlms", "_resources", "_dataStore", "_registry",
        "_event", "_scriptingState", "_settings", "_plugin", "_lottie",
        //The worker namespace itself belongs to the main vm - a worker cannot spawn workers.
        "_worker",
        //Reaches the window through BaseSingleton.
        "_shutdownEngine",
    };

    for(const std::string& name : forbidden){
        ASSERT_FALSE(rootHas(name)) << name << " must not exist in a script worker vm";
    }
}

TEST_F(WorkerVMFixture, noUserdataTypesAreRegistered){
    //Every delegate table in the engine is a single static bound to the main vm, so none of these
    //constructors may exist here. See the comment in ScriptWorkerVM.h.
    const std::vector<std::string> forbidden = { "Vec3", "Vec2", "Quat", "Timer", "ColourValue" };

    for(const std::string& name : forbidden){
        ASSERT_FALSE(rootHas(name)) << name << " must not exist in a script worker vm";
    }
}

TEST_F(WorkerVMFixture, theStandardLibrariesArePresent){
    ASSERT_TRUE(evalBool("typeof abs == \"function\""));
    ASSERT_TRUE(evalBool("typeof format == \"function\""));
    ASSERT_TRUE(evalBool("typeof blob == \"class\""));
    //Data processing without sqrt or sin would be a strange offering.
    ASSERT_TRUE(evalBool("sqrt(4.0) == 2.0"));
}

TEST_F(WorkerVMFixture, fileLoadingAndJsonArePresent){
    ASSERT_TRUE(rootHas("_doFile"));
    ASSERT_TRUE(rootHas("_doFileWithContext"));
    ASSERT_TRUE(rootHas("_compileBuffer"));
    ASSERT_TRUE(rootHas("_time"));
    ASSERT_TRUE(rootHas("_prettyPrint"));

    ASSERT_TRUE(evalBool("typeof _system.readJSONAsTable == \"function\""));
    ASSERT_TRUE(evalBool("typeof _system.writeJsonAsFile == \"function\""));
}

TEST_F(WorkerVMFixture, systemOmitsEverythingWhichMutatesTheFilesystem){
    //Two threads racing over mkdir/remove is not something the engine should make easy.
    const std::vector<std::string> forbidden = { "mkdir", "remove", "removeAll", "rename", "createBlankFile", "getFilesInDirectory" };

    for(const std::string& name : forbidden){
        ASSERT_FALSE(evalBool("\"" + name + "\" in _system")) << "_system." << name << " must not exist in a worker vm";
    }
}

TEST_F(WorkerVMFixture, randomOmitsTheUserdataAndProcessGlobalFunctions){
    ASSERT_TRUE(evalBool("\"rand\" in _random"));
    ASSERT_TRUE(evalBool("\"randInt\" in _random"));
    ASSERT_TRUE(evalBool("\"randIndex\" in _random"));
    ASSERT_TRUE(evalBool("\"seed\" in _random"));

    //These return userdata, which this vm has none of.
    ASSERT_FALSE(evalBool("\"randVec3\" in _random"));
    ASSERT_FALSE(evalBool("\"randVec2\" in _random"));
    ASSERT_FALSE(evalBool("\"randAABB\" in _random"));
    //These go through PatternHelper's process wide seed.
    ASSERT_FALSE(evalBool("\"genPerlinNoise\" in _random"));
    ASSERT_FALSE(evalBool("\"seedPatternGenerator\" in _random"));
}

TEST_F(WorkerVMFixture, workerSelfIsPresentWithItsTwoFunctions){
    ASSERT_TRUE(evalBool("typeof _workerSelf.setProgress == \"function\""));
    ASSERT_TRUE(evalBool("typeof _workerSelf.isCancelled == \"function\""));
}

TEST_F(WorkerVMFixture, theExecutionConstantsSayWhichVmThisIs){
    //A file shared with the main vm branches on these.
    ASSERT_TRUE(evalBool("EXECUTION_WORKER_VM == 1"));
    ASSERT_TRUE(evalBool("EXECUTION_SETUP_VM == 0"));
}

TEST_F(WorkerVMFixture, theStateConstantsAreDeclared){
    ASSERT_TRUE(evalBool("_WORKER_IDLE == 0"));
    ASSERT_TRUE(evalBool("_WORKER_DISPATCHED == 1"));
    ASSERT_TRUE(evalBool("_WORKER_RUNNING == 2"));
    ASSERT_TRUE(evalBool("_WORKER_READY == 3"));
    ASSERT_TRUE(evalBool("_WORKER_FAILED == 4"));
    ASSERT_TRUE(evalBool("_WORKER_DESTROYED == 5"));
}

TEST_F(WorkerVMFixture, theRootTableIsTheOnlyThingLeftOnTheStack){
    //PEG's equivalent pushes the root table and never pops it, so its vm permanently carries a
    //stack entry. create() must leave the stack clean.
    ASSERT_EQ(sq_gettop(vm), 0);
}
