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

    AV::AnimationDefConstructionInfo constructionInfo;
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

    AV::AnimationDefConstructionInfo constructionInfo;
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

    AV::AnimationDefConstructionInfo constructionInfo;
    AV::AnimationScriptParser p;
    bool result = p.parseBuffer(xmlValue, constructionInfo, &logger);
    ASSERT_TRUE(result);

    ASSERT_TRUE(constructionInfo.animInfoHash & AV::ANIM_INFO_SCENE_NODE);
    ASSERT_TRUE(constructionInfo.animInfoHash >> 4 & AV::ANIM_INFO_SCENE_NODE);
    //Everything else should be 0.
    ASSERT_TRUE(constructionInfo.animInfoHash >> 8 == AV::ANIM_INFO_NONE);

    //TESTS -
    //Check if type is not a string or non-existant the value is skipped over meaning later values should be considered.
}

TEST_F(AnimationScriptParserTests, readsAnimations){
    const char* xmlValue = " \
    <AnimationSequence> \
        <data> \
            <targetNode type='SceneNode'/> \
        </data> \
        <animations> \
            <run repeat='true' end='30'> \
                <t type='transform' target='0'></t> \
                <t type='transform' target='0'></t> \
            </run> \
            <walk repeat='true' end='30'> \
                <t type='transform' target='0'></t> \
            </walk> \
        </animations> \
    </AnimationSequence> \
    ";

    AV::AnimationDefConstructionInfo constructionInfo;
    AV::AnimationScriptParser p;
    bool result = p.parseBuffer(xmlValue, constructionInfo, &logger);
    ASSERT_TRUE(result);
}
