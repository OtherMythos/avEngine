#include "gtest/gtest.h"
#include "iostream"

#include "Logger/Log.h"
#include "Scripting/ScriptTestHelper.h"

int main(int argc, char **argv){
    AV::Log::Init();
    ScriptTestHelper::initialise();


    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
