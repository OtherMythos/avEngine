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

TEST_F(AnimationScriptParserTests, parsesKeyframes){
    const char* xmlValue = " \
    <AnimationSequence> \
        <data> \
            <targetNode type='SceneNode'/> \
        </data> \
        <animations> \
            <run repeat='true' end='20'> \
                <t type='transform' target='0'> \
                    <k t='10' position='10, 20, 30'/> \
                    <k t='20' position='40, 50, 60' scale='1, 2, 3'/> \
                </t> \
            </run> \
        </animations> \
    </AnimationSequence> \
    ";

    AV::AnimationParserOutput constructionInfo;
    AV::AnimationScriptParser p;
    bool result = p.parseBuffer(xmlValue, constructionInfo, &logger);
    ASSERT_TRUE(result);

    ASSERT_EQ(constructionInfo.trackDefinition.size(), 1);
    ASSERT_EQ(constructionInfo.keyframes.size(), 2);
    ASSERT_EQ(constructionInfo.animInfo.size(), 1);

    ASSERT_EQ(constructionInfo.keyframes[0].keyframePos, 10);
    ASSERT_EQ(constructionInfo.keyframes[1].keyframePos, 20);

    ASSERT_TRUE(constructionInfo.keyframes[0].data & AV::KeyframeTransformTypes::Position);
    ASSERT_FALSE(constructionInfo.keyframes[0].data & AV::KeyframeTransformTypes::Scale);
    ASSERT_TRUE(constructionInfo.keyframes[1].data & AV::KeyframeTransformTypes::Position);
    ASSERT_TRUE(constructionInfo.keyframes[1].data & AV::KeyframeTransformTypes::Scale);

    ASSERT_EQ(constructionInfo.data.size(), 9);

    AV::uint32 key0Start = constructionInfo.keyframes[0].a.ui;
    ASSERT_EQ(key0Start, 0);
    ASSERT_EQ(constructionInfo.data[key0Start+0], 10.0f);
    ASSERT_EQ(constructionInfo.data[key0Start+1], 20.0f);
    ASSERT_EQ(constructionInfo.data[key0Start+2], 30.0f);

    AV::uint32 key1Start = constructionInfo.keyframes[1].a.ui;
    ASSERT_EQ(key1Start, 3);
    ASSERT_EQ(constructionInfo.data[key1Start+0], 40.0f);
    ASSERT_EQ(constructionInfo.data[key1Start+1], 50.0f);
    ASSERT_EQ(constructionInfo.data[key1Start+2], 60.0f);

    AV::uint32 key1BStart = constructionInfo.keyframes[1].b.ui;
    ASSERT_EQ(key1BStart, 6);
    ASSERT_EQ(constructionInfo.data[key1BStart+0], 1.0f);
    ASSERT_EQ(constructionInfo.data[key1BStart+1], 2.0f);
    ASSERT_EQ(constructionInfo.data[key1BStart+2], 3.0f);
}

TEST_F(AnimationScriptParserTests, parseDetailMapKeyframes){
    const char* xmlValue = " \
    <AnimationSequence> \
        <data> \
            <targetNode type='SceneNode'/> \
        </data> \
        <animations> \
            <run repeat='true' end='20'> \
                <t type='pbsDetailMap' target='0' detailMap='2'> \
                    <k t='0' offset='0, 0' scale='0, 0' weight='10' normWeight='20'/> \
                </t> \
            </run> \
        </animations> \
    </AnimationSequence> \
    ";

    AV::AnimationParserOutput constructionInfo;
    AV::AnimationScriptParser p;
    bool result = p.parseBuffer(xmlValue, constructionInfo, &logger);
    ASSERT_TRUE(result);

    ASSERT_EQ(constructionInfo.trackDefinition.size(), 1);
    ASSERT_EQ(constructionInfo.keyframes.size(), 1);
    ASSERT_EQ(constructionInfo.animInfo.size(), 1);

    AV::uint32 key0Start = constructionInfo.keyframes[0].a.ui;
    ASSERT_EQ(key0Start, 0);
    ASSERT_EQ(constructionInfo.data[key0Start+0], 0.0f);
    ASSERT_EQ(constructionInfo.data[key0Start+1], 0.0f);
    ASSERT_EQ(constructionInfo.data[key0Start+2], 0.0f);
    ASSERT_EQ(constructionInfo.data[key0Start+3], 0.0f);
    ASSERT_EQ(constructionInfo.keyframes[0].b.f, 10.0f);
    ASSERT_EQ(constructionInfo.keyframes[0].c.f, 20.0f);
}

TEST_F(AnimationScriptParserTests, parseDetailMapKeyframeData){
    const char* xmlValue = " \
    <AnimationSequence> \
        <data> \
            <targetNode type='SceneNode'/> \
        </data> \
        <animations> \
            <run repeat='true' end='20'> \
                <t type='pbsDetailMap' target='0' detailMap='2'> \
                    <k t='0' offset='1, 2'/> \
                    <k t='0' offset='2, 3' scale='4, 5'/> \
                    <k t='0' scale='6, 7'/> \
                </t> \
            </run> \
        </animations> \
    </AnimationSequence> \
    ";

    AV::AnimationParserOutput constructionInfo;
    AV::AnimationScriptParser p;
    bool result = p.parseBuffer(xmlValue, constructionInfo, &logger);
    ASSERT_TRUE(result);

    ASSERT_EQ(constructionInfo.trackDefinition.size(), 1);
    ASSERT_EQ(constructionInfo.keyframes.size(), 3);
    ASSERT_EQ(constructionInfo.animInfo.size(), 1);

    ASSERT_EQ(constructionInfo.data.size(), 8);

    AV::uint32 key0Start = constructionInfo.keyframes[0].a.ui;
    ASSERT_EQ(key0Start, 0);
    ASSERT_EQ(constructionInfo.data[key0Start+0], 1.0f);
    ASSERT_EQ(constructionInfo.data[key0Start+1], 2.0f);

    AV::uint32 key1Start = constructionInfo.keyframes[1].a.ui;
    ASSERT_EQ(key1Start, 2);
    ASSERT_EQ(constructionInfo.data[key1Start+0], 2.0f);
    ASSERT_EQ(constructionInfo.data[key1Start+1], 3.0f);
    ASSERT_EQ(constructionInfo.data[key1Start+2], 4.0f);
    ASSERT_EQ(constructionInfo.data[key1Start+3], 5.0f);

    AV::uint32 key2Start = constructionInfo.keyframes[2].a.ui;
    ASSERT_EQ(key2Start, 6);
    ASSERT_EQ(constructionInfo.data[key2Start+0], 6.0f);
    ASSERT_EQ(constructionInfo.data[key2Start+1], 7.0f);
}

TEST_F(AnimationScriptParserTests, readMultiplePbsDetailMapTracks){
    const char* xmlValue = " \
    <AnimationSequence> \
        <data> \
            <targetNode type='SceneNode'/> \
        </data> \
        <animations> \
            <run repeat='true' end='20'> \
                <t type='pbsDetailMap' target='0' detailMap='2'> \
                    <k t='0' offset='0.1, 0.2' weight='1.1' normWeight='1.2'/> \
                    <k t='360' offset='0.3, 0.4' weight='0.1' normWeight='0.2'/> \
                </t> \
                <t type='pbsDetailMap' target='0' detailMap='1'> \
                    <k t='0' offset='0.7, 0.8' weight='0.1' normWeight='1'/> \
                    <k t='360' offset='0.9, 1.1' weight='1' normWeight='0.1'/> \
                </t> \
            </run> \
        </animations> \
    </AnimationSequence> \
    ";

    AV::AnimationParserOutput constructionInfo;
    AV::AnimationScriptParser p;
    bool result = p.parseBuffer(xmlValue, constructionInfo, &logger);
    ASSERT_TRUE(result);

    ASSERT_EQ(constructionInfo.trackDefinition.size(), 2);
    ASSERT_EQ(constructionInfo.keyframes.size(), 4);
    ASSERT_EQ(constructionInfo.animInfo.size(), 1);

    ASSERT_EQ(constructionInfo.data.size(), 8);

    ASSERT_EQ(constructionInfo.trackDefinition[0].userData, 2);
    ASSERT_EQ(constructionInfo.trackDefinition[1].userData, 1);

    AV::uint32 keyStart = constructionInfo.keyframes[0].a.ui;
    ASSERT_EQ(keyStart, 0);
    ASSERT_EQ(constructionInfo.data[keyStart+0], 0.1f);
    ASSERT_EQ(constructionInfo.data[keyStart+1], 0.2f);
    ASSERT_EQ(constructionInfo.keyframes[0].b.f, 1.1f);
    ASSERT_EQ(constructionInfo.keyframes[0].c.f, 1.2f);

    keyStart = constructionInfo.keyframes[1].a.ui;
    ASSERT_EQ(keyStart, 2);
    ASSERT_EQ(constructionInfo.data[keyStart+0], 0.3f);
    ASSERT_EQ(constructionInfo.data[keyStart+1], 0.4f);
    ASSERT_EQ(constructionInfo.keyframes[1].b.f, 0.1f);
    ASSERT_EQ(constructionInfo.keyframes[1].c.f, 0.2f);

    keyStart = constructionInfo.keyframes[2].a.ui;
    ASSERT_EQ(keyStart, 4);
    ASSERT_EQ(constructionInfo.data[keyStart+0], 0.7f);
    ASSERT_EQ(constructionInfo.data[keyStart+1], 0.8f);
    ASSERT_EQ(constructionInfo.keyframes[2].b.f, 0.1f);
    ASSERT_EQ(constructionInfo.keyframes[2].c.f, 1.0f);

    keyStart = constructionInfo.keyframes[3].a.ui;
    ASSERT_EQ(keyStart, 6);
    ASSERT_EQ(constructionInfo.data[keyStart+0], 0.9f);
    ASSERT_EQ(constructionInfo.data[keyStart+1], 1.1f);
    ASSERT_EQ(constructionInfo.keyframes[3].b.f, 1.0f);
    ASSERT_EQ(constructionInfo.keyframes[3].c.f, 0.1f);
}

TEST_F(AnimationScriptParserTests, multipleAnimations){
    const char* xmlValue = " \
        <AnimationSequence> \
            <data> \
                <targetNode type='SceneNode'/> \
            </data> \
            <animations> \
                <exploderHelecopter repeat='true' end='10'> \
                    <t type='transform' target='0'> \
                        <k t='0' rot='0, 0, 0'/> \
                        <k t='10' rot='0, 90, 0'/> \
                    </t> \
                </exploderHelecopter> \
                <leaveGround repeat='false' end='30'> \
                    <t type='transform' target='0'> \
                        <k t='0' position='0, 0, 0' scale='0, 0, 0'/> \
                        <k t='30' position='0, 3, 0' scale='1, 1, 1'/> \
                    </t> \
                </leaveGround> \
                <enterGround repeat='false' end='30'> \
                    <t type='transform' target='0'> \
                        <k t='0' position='0, 3, 0' scale='1, 1, 1'/> \
                        <k t='30' position='0, 0, 0' scale='0, 0, 0'/> \
                    </t> \
                </enterGround> \
            </animations> \
        </AnimationSequence> \
    ";

    AV::AnimationParserOutput constructionInfo;
    AV::AnimationScriptParser p;
    bool result = p.parseBuffer(xmlValue, constructionInfo, &logger);
    ASSERT_TRUE(result);

    ASSERT_EQ(constructionInfo.trackDefinition.size(), 3);
    ASSERT_EQ(constructionInfo.keyframes.size(), 6);
    ASSERT_EQ(constructionInfo.animInfo.size(), 3);

    //rot entries boil down to a quaternion which pushes four values rather than 3.
    ASSERT_EQ(constructionInfo.data.size(), 30);

    ASSERT_EQ(constructionInfo.trackDefinition[0].keyframeStart, 0);
    ASSERT_EQ(constructionInfo.trackDefinition[0].keyframeEnd, 2);
    ASSERT_EQ(constructionInfo.trackDefinition[1].keyframeStart, 0);
    ASSERT_EQ(constructionInfo.trackDefinition[1].keyframeEnd, 2);
    ASSERT_EQ(constructionInfo.trackDefinition[2].keyframeStart, 0);
    ASSERT_EQ(constructionInfo.trackDefinition[2].keyframeEnd, 2);
}

TEST_F(AnimationScriptParserTests, multipleAnimationData){
    const char* xmlValue = " \
        <AnimationSequence> \
            <data> \
                <firstNode type='SceneNode'/> \
                <secondNode type='SceneNode'/> \
            </data> \
            <data> \
                <targetNode type='SceneNode'/> \
            </data> \
            <animations> \
                <firstAnim repeat='true' end='10' data='0'> \
                    <t type='transform' target='0'> \
                        <k t='0' rot='0, 0, 0'/> \
                        <k t='10' rot='0, 90, 0'/> \
                    </t> \
                </firstAnim> \
            </animations> \
        </AnimationSequence> \
    ";

    AV::AnimationParserOutput constructionInfo;
    AV::AnimationScriptParser p;
    bool result = p.parseBuffer(xmlValue, constructionInfo, &logger);
    ASSERT_TRUE(result);

    ASSERT_EQ(constructionInfo.infoHashes.size(), 2);

    ASSERT_TRUE(constructionInfo.infoHashes[0] & AV::ANIM_INFO_SCENE_NODE);
    ASSERT_TRUE(constructionInfo.infoHashes[0] >> 4 & AV::ANIM_INFO_SCENE_NODE);

    ASSERT_TRUE(constructionInfo.infoHashes[1] & AV::ANIM_INFO_SCENE_NODE);
    ASSERT_TRUE(constructionInfo.infoHashes[1] >> 4 == AV::ANIM_INFO_NONE);
}

TEST_F(AnimationScriptParserTests, producesCorrectKeyframeSkipList){

    AV::AnimationScriptParser p;

    {
        AV::uint8 outValues[3];
        const std::vector<AV::Keyframe> vec({
            {0, 0, 0, 0},
            {30, 0, 0, 0},
        });
        p._produceKeyframeSkipMap(vec, 60, 0, vec.size()-1, outValues);

        ASSERT_EQ(outValues[0], 0); //25%
        ASSERT_EQ(outValues[1], 1); //50%
        ASSERT_EQ(outValues[2], 1); //75%
    }

    {
        AV::uint8 outValues[3];
        const std::vector<AV::Keyframe> vec({
            {0, 0, 0, 0},
            {15, 0, 0, 0},
            {30, 0, 0, 0},
            {45, 0, 0, 0},
            {60, 0, 0, 0},
        });
        p._produceKeyframeSkipMap(vec, 60, 0, vec.size()-1, outValues);

        ASSERT_EQ(outValues[0], 1);
        ASSERT_EQ(outValues[1], 2);
        ASSERT_EQ(outValues[2], 3);
    }

    {
        AV::uint8 outValues[3];
        const std::vector<AV::Keyframe> vec({
            {0, 0, 0, 0},
            {8, 0, 0, 0},
            {90, 0, 0, 0}
        });
        p._produceKeyframeSkipMap(vec, 120, 0, vec.size()-1, outValues);

        ASSERT_EQ(outValues[0], 1);
        ASSERT_EQ(outValues[1], 1);
        ASSERT_EQ(outValues[2], 2);
    }

    {
        AV::uint8 outValues[3];
        const std::vector<AV::Keyframe> vec({
            {0, 0, 0, 0},
            {8, 0, 0, 0},
            {10, 0, 0, 0},
            {15, 0, 0, 0},
            {20, 0, 0, 0},
        });
        p._produceKeyframeSkipMap(vec, 120, 0, vec.size()-1, outValues);

        ASSERT_EQ(outValues[0], 4);
        ASSERT_EQ(outValues[1], 4);
        ASSERT_EQ(outValues[2], 4);
    }

    {
        AV::uint8 outValues[3];
        const std::vector<AV::Keyframe> vec({
            {0, 0, 0, 0},
            {8, 0, 0, 0},
            {10, 0, 0, 0},
            {15, 0, 0, 0},
            {20, 0, 0, 0},
            {30, 0, 0, 0},
        });
        p._produceKeyframeSkipMap(vec, 120, 0, vec.size()-1, outValues);

        ASSERT_EQ(outValues[0], 5);
        ASSERT_EQ(outValues[1], 5);
        ASSERT_EQ(outValues[2], 5);
    }

    {
        AV::uint8 outValues[3];
        const std::vector<AV::Keyframe> vec({
            {0, 0, 0, 0},
            {8, 0, 0, 0},
            {10, 0, 0, 0},
            {15, 0, 0, 0},
            {20, 0, 0, 0},
            {30, 0, 0, 0},

            {60, 0, 0, 0},
            {61, 0, 0, 0},

            {90, 0, 0, 0},
        });
        p._produceKeyframeSkipMap(vec, 120, 0, vec.size()-1, outValues);

        ASSERT_EQ(outValues[0], 5);
        ASSERT_EQ(outValues[1], 6);
        ASSERT_EQ(outValues[2], 8);
    }

    {
        AV::uint8 outValues[3];
        const std::vector<AV::Keyframe> vec({
            {0, 0, 0, 0},
            {8, 0, 0, 0},
            {10, 0, 0, 0},
            {15, 0, 0, 0},
            {20, 0, 0, 0},
            {30, 0, 0, 0},

            {60, 0, 0, 0},
            {61, 0, 0, 0},

            {90, 0, 0, 0},
        });
        p._produceKeyframeSkipMap(vec, 120, 0, vec.size()-1, outValues);

        ASSERT_EQ(outValues[0], 5);
        ASSERT_EQ(outValues[1], 6);
        ASSERT_EQ(outValues[2], 8);
    }

    {
        AV::uint8 outValues[3];
        const std::vector<AV::Keyframe> vec({
            {0, 0, 0, 0},
            {0, 0, 0, 0},
            {0, 0, 0, 0},
            {0, 0, 0, 0},
        });
        p._produceKeyframeSkipMap(vec, 120, 0, vec.size()-1, outValues);

        ASSERT_EQ(outValues[0], 3);
        ASSERT_EQ(outValues[1], 3);
        ASSERT_EQ(outValues[2], 3);
    }

    {
        AV::uint8 outValues[3];
        const std::vector<AV::Keyframe> vec({
            {0, 0, 0, 0},
            {0, 0, 0, 0},
            {0, 0, 0, 0},
            {30, 0, 0, 0},
        });
        p._produceKeyframeSkipMap(vec, 40, 0, vec.size()-1, outValues);

        ASSERT_EQ(outValues[0], 2);
        ASSERT_EQ(outValues[1], 2);
        ASSERT_EQ(outValues[2], 3);
    }
}

TEST_F(AnimationScriptParserTests, producesCorrectKeyframeSkipListOddNumbers){
    AV::AnimationScriptParser p;
    {
        AV::uint8 outValues[3];
        const std::vector<AV::Keyframe> vec({
            {0, 0, 0, 0},
            {15, 0, 0, 0},
            {27, 0, 0, 0},
            {30, 0, 0, 0},
        });
        p._produceKeyframeSkipMap(vec, 35, 0, vec.size()-1, outValues);

        ASSERT_EQ(outValues[0], 0);
        ASSERT_EQ(outValues[1], 1);
        ASSERT_EQ(outValues[2], 1);
    }
}

TEST_F(AnimationScriptParserTests, producesCorrectKeyframeWithMultipleAnim){
    AV::AnimationScriptParser p;
    {
        AV::uint8 outValues[3];
        const std::vector<AV::Keyframe> vec({
            {0, 0, 0, 0},
            {15, 0, 0, 0},
            {0, 0, 0, 0},
            {15, 0, 0, 0},
        });
        p._produceKeyframeSkipMap(vec, 30, 0, 1, outValues);

        ASSERT_EQ(outValues[0], 0);
        ASSERT_EQ(outValues[1], 0);
        ASSERT_EQ(outValues[2], 1);

        p._produceKeyframeSkipMap(vec, 30, 2, 3, outValues);

        ASSERT_EQ(outValues[0], 0);
        ASSERT_EQ(outValues[1], 0);
        ASSERT_EQ(outValues[2], 1);
    }
}

//TESTS -
//Check if type is not a string or non-existant the value is skipped over meaning later values should be considered.
//Animatios reference data 0 by default. What if it's not there? Also check it does do that by default, and also not if overriden.
