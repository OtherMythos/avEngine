#include "gtest/gtest.h"
#include "iostream"

#include "Logger/Log.h"
#include "Scripting/ScriptTestHelper.h"

int main(int argc, char **argv){
    AV::Log::Init();
    //Keep the test runner output clean; file logging is still written.
    AV::Log::DisableTerminalOutput();
    ScriptTestHelper::initialise();


    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
