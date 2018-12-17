#include "gtest/gtest.h"
#include "iostream"

#include "Logger/Log.h"

int main(int argc, char **argv){
    AV::Log::Init();
    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
