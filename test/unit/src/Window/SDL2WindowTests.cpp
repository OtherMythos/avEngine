#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Window/SDL2Window/SDL2Window.h"
#include <memory>
#include "Logger/Log.h"

class MockSDL2Window : public AV::SDL2Window{
public:
    MOCK_METHOD0(isOpen, bool());
    MOCK_METHOD0(close, bool());
    MOCK_METHOD2(_handleKey, void(SDL_Keysym key, bool pressed));
    
    void handleEvent(SDL_Event &event) { _handleEvent(event); };
    void handleKey(SDL_Keysym key, bool pressed) { _handleKey(key, pressed); };
};

class testSDL2Window : public AV::SDL2Window{
public:
    void testSetOpen(bool val) {_open = val; };
};

TEST(SDL2WindowTests, closeReturnsFalseIfWindowNotOpen){
//    std::unique_ptr<AV::SDL2Window> window(new AV::SDL2Window());
//
//    bool result = window->close();
//    EXPECT_EQ(result, false);
}

TEST(SDL2WindowTests, checkOpenReturnsFalseIfOpen){
//    testSDL2Window window;
//    window.testSetOpen(true);
//
//    bool val = window.open();
//    ASSERT_EQ(val, false);
}

TEST(SDL2WindowTests, quitEventCausesClose){
//    MockSDL2Window window;
//    EXPECT_CALL(window, close())
//    .Times(testing::AtLeast(1));
//    
//    SDL_Event event;
//    event.type = SDL_QUIT;
//    window.handleEvent(event);
//    
}

TEST(SDL2WindowTests, keyPressedEventCausesHandleKey){
//    MockSDL2Window window;
//    
//    SDL_Event event;
//    SDL_KeyboardEvent kevent;
//    event.type = SDL_KEYDOWN;
//    kevent.type = SDL_KEYDOWN;
//    kevent.repeat = 0;
//    event.key = kevent;
//    
//    window.handleEvent(event);
}
