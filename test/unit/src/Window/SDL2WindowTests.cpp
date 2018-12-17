#include "gtest/gtest.h"

#include "Window/SDL2Window/SDL2Window.h"

TEST(SDL2WindowTests, closeReturnsFalseIfWindowNotOpen){
    AV::SDL2Window *window = new AV::SDL2Window();
    
    bool result = window->close();
    EXPECT_EQ(result, false);
    
    delete window;
}
