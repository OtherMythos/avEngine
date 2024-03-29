#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include <fstream>
#include "World/Slot/Recipe/CollisionObjectSceneParser.h"
#include "World/Physics/Worlds/CollisionWorldUtils.h"

class CollisionObjectSceneParserTests : public ::testing::Test {
private:
    AV::RecipeData recipeData;
    AV::CollisionObjectSceneParser parser;
public:
    CollisionObjectSceneParserTests() {

    }

    virtual ~CollisionObjectSceneParserTests() {

    }

    virtual void SetUp() {
        parser._populateRecipeData(recipeData.collisionData);
    }

    virtual void TearDown() {
        parser._clearRecipeData(recipeData.collisionData);
    }


    //writes contents to the target path.
    const char* prepareSceneFile(const char* contents){
        const char* targetPath = "/tmp/data.txt";
        std::ofstream outfile;
        outfile.open(targetPath);

        outfile << contents << std::endl;

        outfile.close();

        return targetPath;
    }
};

TEST_F(CollisionObjectSceneParserTests, ParserReadsCorrectData){
    const char* file = prepareSceneFile(
        "0\n"
        "1.123 20 40.12\n"
        "==\n"
        "res://first.nut\n"
        "==\n"
        "Something\n"
        "SomethingElse\n"
        "==\n"
        "0 0\n"
        "0 1\n"
        "==\n"
        "0\n"
        "1001010\n"
        "100\n"
        "0\n"
        "==\n"
        "0\n"
        "0\n"
        "0\n"
        "10 20 30.10\n"
        "0 0 0 1\n"
    );

    ASSERT_TRUE(parser._parse(file, recipeData.collisionData));
    const AV::CollisionWorldChunkData& data = recipeData.collisionData;

    //Should have found a single shape.
    ASSERT_EQ(1, data.collisionShapeData->size());
    ASSERT_EQ(btVector3(1.123, 20, 40.12), (*data.collisionShapeData)[0].scale);
    ASSERT_EQ(0, (*data.collisionShapeData)[0].physicsShapeType);

    //Should find two closurs and one script path.
    ASSERT_EQ(3, data.collisionScriptAndClosures->size());
    ASSERT_EQ(1, data.collisionClosuresBegin);
    ASSERT_EQ("res://first.nut", (*data.collisionScriptAndClosures)[0] );
    ASSERT_EQ("Something", (*data.collisionScriptAndClosures)[1] );
    ASSERT_EQ("SomethingElse", (*data.collisionScriptAndClosures)[2] );

    ASSERT_EQ(2, data.collisionScriptData->size() );
    ASSERT_EQ(0, (*data.collisionScriptData)[0].scriptIdx );
    ASSERT_EQ(0, (*data.collisionScriptData)[0].scriptIdx );
    ASSERT_EQ(0, (*data.collisionScriptData)[1].scriptIdx );
    ASSERT_EQ(0, (*data.collisionScriptData)[0].closureIdx );
    ASSERT_EQ(1, (*data.collisionScriptData)[1].closureIdx );

    ASSERT_EQ(1, data.collisionObjectPackedData->size());
    AV::CollisionPackedInt packedData = (*data.collisionObjectPackedData)[0].packedInt;

    AV::CollisionWorldUtils::PackedIntContents outContents;
    AV::CollisionWorldUtils::readPackedInt(packedData, &outContents);
    ASSERT_EQ(0, outContents.worldId);

    ASSERT_TRUE(outContents.target & AV::CollisionObjectTypeMask::PLAYER > 0);
    ASSERT_TRUE(outContents.target & AV::CollisionObjectTypeMask::USER_3 > 0);
    ASSERT_TRUE(outContents.target & AV::CollisionObjectTypeMask::USER_5 > 0);
    ASSERT_EQ(0, outContents.target & AV::CollisionObjectTypeMask::ENEMY);
    ASSERT_EQ(0, outContents.target & AV::CollisionObjectTypeMask::OBJECT);
    ASSERT_EQ(0, outContents.target & AV::CollisionObjectTypeMask::USER_4);
    ASSERT_EQ(0, outContents.target & AV::CollisionObjectTypeMask::USER_6);

    ASSERT_TRUE(outContents.eventType & AV::CollisionObjectEventMask::LEAVE > 0);
    ASSERT_EQ(0, outContents.eventType & AV::CollisionObjectEventMask::ENTER);
    ASSERT_EQ(0, outContents.eventType & AV::CollisionObjectEventMask::INSIDE);

    ASSERT_EQ(1, data.collisionObjectRecipeData->size());
    ASSERT_EQ(btVector3(10, 20, 30.10), (*data.collisionObjectRecipeData)[0].pos);
    ASSERT_EQ(btQuaternion(0, 0, 0, 1), (*data.collisionObjectRecipeData)[0].orientation);
}

TEST_F(CollisionObjectSceneParserTests, ParserReadsCorrectDataWithInvalidScript){
    //The parser should not stop parsing if an empty script or closure is found.
    const char* file = prepareSceneFile(
        "0\n"
        "1.123 20 40.12\n"
        "==\n"
        "\n" //Empty script
        "==\n"
        "\n" //Empty closure
        "==\n"
        "0 0\n"
        "==\n"
        "0\n"
        "1001010\n"
        "010\n"
        "0\n"
        "==\n"
        "0\n"
        "0\n"
        "0\n"
        "10 20 30.10\n"
        "0 0 0 1\n"
    );

    ASSERT_TRUE(parser._parse(file, recipeData.collisionData));
    const AV::CollisionWorldChunkData& data = recipeData.collisionData;

    //Even though they're not valid paths they should still be parsed.
    ASSERT_EQ(2, data.collisionScriptAndClosures->size());
    ASSERT_EQ(1, data.collisionClosuresBegin);
}
