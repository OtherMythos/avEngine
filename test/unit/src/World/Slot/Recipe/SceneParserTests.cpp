#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include <fstream>
#include "World/Slot/Recipe/SceneParser.h"

class SceneParserTests : public ::testing::Test {
private:
    AV::RecipeData data;
    AV::SceneParser parser;
public:
    SceneParserTests() {

    }

    virtual ~SceneParserTests() {

    }

    virtual void SetUp() {
        parser._populateRecipeData(&data);
    }

    virtual void TearDown() {
        parser._clearRecipeData(&data);
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

TEST_F(SceneParserTests, SceneParserParsesNodes){
    const char* file = prepareSceneFile(
        "0 0 0\n"
        "0 0 0\n"
        "0 1 0\n"
        "10 20 30\n" //This line should not get added to the list.
    );

    ASSERT_TRUE(parser._parseSceneTreeFile(file, &data));

    ASSERT_EQ(data.sceneEntries->size(), 3);
    ASSERT_EQ((*(data.sceneEntries))[2].pos, Ogre::Vector3(10, 20, 30));
    ASSERT_EQ((*(data.sceneEntries))[0].pos, Ogre::Vector3::ZERO);
}

TEST_F(SceneParserTests, SceneParserParsesTermNodes){
    const char* file = prepareSceneFile(
        "0 0 0\n"
        "1 0 0\n"
        "3 0 0\n"
        "2 0 0\n"
        "0 0 0\n"
    );

    ASSERT_TRUE(parser._parseSceneTreeFile(file, &data));

    ASSERT_EQ(data.sceneEntries->size(), 5);
    AV::SceneType types[5] = {
        AV::SceneType::empty,
        AV::SceneType::child,
        AV::SceneType::mesh,
        AV::SceneType::term,
        AV::SceneType::empty
    };
    for(int i = 0; i < 5; i++){
        ASSERT_EQ((*(data.sceneEntries))[i].type, types[i]);
    }
}

TEST_F(SceneParserTests, SceneParserParsesStaticMeshes){
    const char* file = prepareSceneFile(
        "ogrehead.mesh\n"
        "ogrehead.mesh\n"
    );

    parser._parseStaticMeshes(file, &data);

    ASSERT_EQ(data.ogreMeshData->size(), 2);
    ASSERT_EQ( (*data.ogreMeshData)[0].meshName, (*data.ogreMeshData)[1].meshName );
}
