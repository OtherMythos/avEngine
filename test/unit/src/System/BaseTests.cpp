#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Logger/Log.h"

#include "System/Base.h"
#include "Window/SDL2Window/SDL2Window.h"

#include <memory>

class MockSDL2Window : public AV::SDL2Window{
public:
    MOCK_METHOD0(open, bool());
};

TEST(BaseTests, initialiseOpensWindow){
    std::shared_ptr<MockSDL2Window> window = std::make_shared<MockSDL2Window>();
    
    EXPECT_CALL(*window, open).Times(testing::AtLeast(1));
    
    AV::Base* base = new AV::Base(window);
    
    delete base;
    EXPECT_TRUE(testing::Mock::VerifyAndClearExpectations(window.get()));
}
