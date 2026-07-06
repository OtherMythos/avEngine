#include "gtest/gtest.h"

//Reach the dispatch's private members (mPassProperties etc.) the same way the
//other suites do.
#define private public
#include "System/OgreSetup/CustomHLMS/AVHlmsListenerDispatch.h"
#undef private

#include "OgreIdString.h"

//A minimal external listener. Real listeners must report 16-byte-multiple
//sizes; this one contributes 16 bytes (4 floats) and writes two sentinels.
class MockPassListener : public Ogre::HlmsListener{
public:
    Ogre::uint32 getPassBufferSize( const Ogre::CompositorShadowNode*, bool, bool, Ogre::SceneManager* ) const override {
        return 16u;
    }
    float* preparePassBuffer( const Ogre::CompositorShadowNode*, bool, bool, Ogre::SceneManager*, float* p ) override {
        p[0] = 101.0f; p[1] = 102.0f; p[2] = 0.0f; p[3] = 0.0f;
        return p + 4;
    }
};

TEST(AVHlmsListenerDispatchTests, passBufferSizeIsPaddedTo16){
    Ogre::AVHlmsListenerDispatch dispatch;

    EXPECT_EQ(dispatch.getPassBufferSize(nullptr, false, false, nullptr), 0u);

    dispatch.setScriptBufferSize(1);
    EXPECT_EQ(dispatch.getPassBufferSize(nullptr, false, false, nullptr), 16u);

    dispatch.setScriptBufferSize(4);
    EXPECT_EQ(dispatch.getPassBufferSize(nullptr, false, false, nullptr), 16u);

    dispatch.setScriptBufferSize(5);
    EXPECT_EQ(dispatch.getPassBufferSize(nullptr, false, false, nullptr), 32u);
}

TEST(AVHlmsListenerDispatchTests, preparePassBufferWritesScriptFloatsThenPad){
    Ogre::AVHlmsListenerDispatch dispatch;
    dispatch.setScriptBufferSize(5);
    for(int i = 0; i < 5; ++i) dispatch.setScriptFloat(i, static_cast<float>(i + 1));

    float buffer[16];
    for(float& f : buffer) f = -1.0f;

    float* end = dispatch.preparePassBuffer(nullptr, false, false, nullptr, buffer);

    //5 floats padded up to 8 (32 bytes).
    EXPECT_EQ(end - buffer, 8);
    EXPECT_EQ(dispatch.getPassBufferSize(nullptr, false, false, nullptr) / 4u, 8u);
    for(int i = 0; i < 5; ++i) EXPECT_FLOAT_EQ(buffer[i], static_cast<float>(i + 1));
    EXPECT_FLOAT_EQ(buffer[5], 0.0f);
    EXPECT_FLOAT_EQ(buffer[6], 0.0f);
    EXPECT_FLOAT_EQ(buffer[7], 0.0f);
}

TEST(AVHlmsListenerDispatchTests, externalListenerFanOutAndOrdering){
    Ogre::AVHlmsListenerDispatch dispatch;
    dispatch.setScriptBufferSize(2);
    dispatch.setScriptFloat(0, 11.0f);
    dispatch.setScriptFloat(1, 12.0f);

    MockPassListener mock;
    dispatch.registerListener(&mock);

    //Script region padded to 16, plus the external's 16 = 32 bytes.
    EXPECT_EQ(dispatch.getPassBufferSize(nullptr, false, false, nullptr), 32u);

    float buffer[16];
    for(float& f : buffer) f = -1.0f;
    float* end = dispatch.preparePassBuffer(nullptr, false, false, nullptr, buffer);

    EXPECT_EQ(end - buffer, 8);
    EXPECT_FLOAT_EQ(buffer[0], 11.0f);   //script floats first
    EXPECT_FLOAT_EQ(buffer[1], 12.0f);
    EXPECT_FLOAT_EQ(buffer[2], 0.0f);    //script region pad to 16
    EXPECT_FLOAT_EQ(buffer[3], 0.0f);
    EXPECT_FLOAT_EQ(buffer[4], 101.0f);  //external contribution, 16-aligned
    EXPECT_FLOAT_EQ(buffer[5], 102.0f);

    dispatch.removeListener(&mock);
    EXPECT_EQ(dispatch.getPassBufferSize(nullptr, false, false, nullptr), 16u);
}

TEST(AVHlmsListenerDispatchTests, passPropertyStorage){
    Ogre::AVHlmsListenerDispatch dispatch;

    dispatch.setPassProperty(20, Ogre::IdString("foo"), 1);
    dispatch.setPassProperty(20, Ogre::IdString("bar"), 2);
    dispatch.setPassProperty(21, Ogre::IdString("baz"), 3);

    ASSERT_EQ(dispatch.mPassProperties.size(), 2u);
    ASSERT_EQ(dispatch.mPassProperties[20].size(), 2u);
    ASSERT_EQ(dispatch.mPassProperties[21].size(), 1u);

    //Overwrite keeps the count but updates the value.
    dispatch.setPassProperty(20, Ogre::IdString("foo"), 9);
    ASSERT_EQ(dispatch.mPassProperties[20].size(), 2u);
    bool foundFoo = false;
    for(const std::pair<Ogre::IdString, Ogre::int32>& p : dispatch.mPassProperties[20]){
        if(p.first == Ogre::IdString("foo")){ foundFoo = true; EXPECT_EQ(p.second, 9); }
    }
    EXPECT_TRUE(foundFoo);

    //Clear removes just the one entry.
    dispatch.clearPassProperty(20, Ogre::IdString("foo"));
    ASSERT_EQ(dispatch.mPassProperties[20].size(), 1u);
    EXPECT_TRUE(dispatch.mPassProperties[20][0].first == Ogre::IdString("bar"));
}
