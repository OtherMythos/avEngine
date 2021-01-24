#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "Animation/Script/AnimationScriptParser.h"

#include <iostream>

class TestScriptLogger : public AV::AnimationScriptParserLogger{
    virtual void notifyError(const std::string& message){
        std::cout << message << std::endl;
    }

    virtual void notifyWarning(const std::string& message){
        std::cout << message << std::endl;
    }
};

class AnimationScriptParserTests : public ::testing::Test {
private:
    TestScriptLogger logger;
public:
    AnimationScriptParserTests() {
    }

    virtual ~AnimationScriptParserTests() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
};

TEST_F(AnimationScriptParserTests, failsOnInvalidXML){
    const char* xmlValue = "Some invalid xml.";

    AV::AnimationParserOutput constructionInfo;
    AV::AnimationScriptParser p;
    bool result = p.parseBuffer(xmlValue, constructionInfo, &logger);
    ASSERT_FALSE(result);
}

TEST_F(AnimationScriptParserTests, parsesSimpleScript){
    const char* xmlValue = " \
    <AnimationSequence> \
        <data> \
            <targetNode type='SceneNode'/> \
        </data> \
        <animations> \
            <run repeat='true' end='30'> \
            </run> \
        </animations> \
    </AnimationSequence> \
    ";

    AV::AnimationParserOutput constructionInfo;
    AV::AnimationScriptParser p;
    bool result = p.parseBuffer(xmlValue, constructionInfo, &logger);
    ASSERT_TRUE(result);
}

TEST_F(AnimationScriptParserTests, findsCorrectAnimationData){
    //NOTE Right now I only have the one type, so this test is slightly invalid.
    const char* xmlValue = " \
    <AnimationSequence> \
        <data> \
            <targetNode type='SceneNode'/> \
            <secondNode type='SceneNode'/> \
        </data> \
    </AnimationSequence> \
    ";

    AV::AnimationParserOutput constructionInfo;
    AV::AnimationScriptParser p;
    bool result = p.parseBuffer(xmlValue, constructionInfo, &logger);
    ASSERT_TRUE(result);

    ASSERT_EQ(constructionInfo.infoHashes.size(), 1);
    ASSERT_TRUE(constructionInfo.infoHashes[0] & AV::ANIM_INFO_SCENE_NODE);
    ASSERT_TRUE(constructionInfo.infoHashes[0] >> 4 & AV::ANIM_INFO_SCENE_NODE);
    //Everything else should be 0.
    ASSERT_TRUE(constructionInfo.infoHashes[0] >> 8 == AV::ANIM_INFO_NONE);

    //TESTS -
    //Check if type is not a string or non-existant the value is skipped over meaning later values should be considered.
    //Animatios reference data 0 by default. What if it's not there? Also check it does do that by default, and also not if overriden.
}

TEST_F(AnimationScriptParserTests, readsAnimations){
    const char* xmlValue = " \
    <AnimationSequence> \
        <data> \
            <targetNode type='SceneNode'/> \
        </data> \
        <animations> \
            <run repeat='true' end='20'> \
                <t type='transform' target='0'></t> \
                <t type='transform' target='0'></t> \
            </run> \
            <walk repeat='false' end='30'> \
                <t type='transform' target='0'></t> \
            </walk> \
        </animations> \
    </AnimationSequence> \
    ";

    AV::AnimationParserOutput constructionInfo;
    AV::AnimationScriptParser p;
    bool result = p.parseBuffer(xmlValue, constructionInfo, &logger);
    ASSERT_TRUE(result);

    ASSERT_EQ(constructionInfo.trackDefinition.size(), 3);
    ASSERT_EQ(constructionInfo.keyframes.size(), 0);
    ASSERT_EQ(constructionInfo.animInfo.size(), 2);

    ASSERT_EQ(constructionInfo.animInfo[0].length, 20);
    ASSERT_TRUE(constructionInfo.animInfo[0].repeats);

    ASSERT_EQ(constructionInfo.animInfo[1].length, 30);
    ASSERT_FALSE(constructionInfo.animInfo[1].repeats);
}
