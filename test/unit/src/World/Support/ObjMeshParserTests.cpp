#include "gtest/gtest.h"

#include "World/Support/Obj/ObjMeshParser.h"

class ObjMeshParserTests : public ::testing::Test{
public:
    AV::ObjMeshData data;
    std::string error;

    bool parse(const std::string& content){
        return AV::ObjMeshParser::parse(content, data, error);
    }

    Ogre::Vector3 position(size_t vertex){
        const float* v = &(data.vertices[vertex * AV::ObjMeshData::ELEMENTS_PER_VERTEX]);
        return Ogre::Vector3(v[0], v[1], v[2]);
    }

    Ogre::Vector3 normal(size_t vertex){
        const float* v = &(data.vertices[vertex * AV::ObjMeshData::ELEMENTS_PER_VERTEX]);
        return Ogre::Vector3(v[3], v[4], v[5]);
    }

    Ogre::Vector2 uv(size_t vertex){
        const float* v = &(data.vertices[vertex * AV::ObjMeshData::ELEMENTS_PER_VERTEX]);
        return Ogre::Vector2(v[6], v[7]);
    }
};

TEST_F(ObjMeshParserTests, parsesSimpleTriangle){
    ASSERT_TRUE(parse(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "f 1 2 3\n"
    ));

    ASSERT_EQ(data.numVertices(), 3u);
    ASSERT_EQ(data.subMeshes.size(), 1u);
    const std::vector<Ogre::uint32> expected{0, 1, 2};
    ASSERT_EQ(data.subMeshes[0].indices, expected);
    ASSERT_EQ(position(0), Ogre::Vector3(0, 0, 0));
    ASSERT_EQ(position(1), Ogre::Vector3(1, 0, 0));
    ASSERT_EQ(position(2), Ogre::Vector3(0, 1, 0));
    ASSERT_TRUE(data.subMeshes[0].materialName.empty());
}

TEST_F(ObjMeshParserTests, parsesFullTripletsAndFlipsUvs){
    ASSERT_TRUE(parse(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 1 1 0\n"
        "vt 0 0\n"
        "vt 1 0.25\n"
        "vt 1 1\n"
        "vn 0 0 1\n"
        "f 1/1/1 2/2/1 3/3/1\n"
    ));

    ASSERT_EQ(data.numVertices(), 3u);
    //The v coordinate should be flipped as obj uses a bottom left origin.
    ASSERT_EQ(uv(0), Ogre::Vector2(0, 1));
    ASSERT_EQ(uv(1), Ogre::Vector2(1, 0.75));
    ASSERT_EQ(uv(2), Ogre::Vector2(1, 0));
    ASSERT_EQ(normal(0), Ogre::Vector3(0, 0, 1));
}

TEST_F(ObjMeshParserTests, parsesNormalOnlyForm){
    ASSERT_TRUE(parse(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "vn 0 1 0\n"
        "f 1//1 2//1 3//1\n"
    ));

    ASSERT_EQ(data.numVertices(), 3u);
    ASSERT_EQ(normal(2), Ogre::Vector3(0, 1, 0));
    ASSERT_EQ(uv(2), Ogre::Vector2(0, 0));
}

TEST_F(ObjMeshParserTests, fanTriangulatesQuadsAndNgons){
    ASSERT_TRUE(parse(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 1 1 0\n"
        "v 0 1 0\n"
        "f 1 2 3 4\n"
    ));

    ASSERT_EQ(data.numVertices(), 4u);
    const std::vector<Ogre::uint32> expected{0, 1, 2, 0, 2, 3};
    ASSERT_EQ(data.subMeshes[0].indices, expected);

    //A pentagon should produce three triangles.
    ASSERT_TRUE(parse(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 1.5 1 0\n"
        "v 0.5 1.5 0\n"
        "v -0.5 1 0\n"
        "f 1 2 3 4 5\n"
    ));
    ASSERT_EQ(data.subMeshes[0].indices.size(), 9u);
}

TEST_F(ObjMeshParserTests, supportsNegativeIndices){
    ASSERT_TRUE(parse(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "f -3 -2 -1\n"
    ));

    ASSERT_EQ(data.numVertices(), 3u);
    ASSERT_EQ(position(0), Ogre::Vector3(0, 0, 0));
    ASSERT_EQ(position(2), Ogre::Vector3(0, 1, 0));
}

TEST_F(ObjMeshParserTests, deduplicatesSharedVertices){
    //Two triangles sharing an edge, with matching triplets.
    ASSERT_TRUE(parse(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 1 1 0\n"
        "v 0 1 0\n"
        "f 1 2 3\n"
        "f 1 3 4\n"
    ));

    ASSERT_EQ(data.numVertices(), 4u);
    ASSERT_EQ(data.subMeshes[0].indices.size(), 6u);
}

TEST_F(ObjMeshParserTests, differingTripletsAreNotDeduplicated){
    //The same position with two different normals should produce two vertices.
    ASSERT_TRUE(parse(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "vn 0 0 1\n"
        "vn 0 1 0\n"
        "f 1//1 2//1 3//1\n"
        "f 1//2 2//1 3//1\n"
    ));

    ASSERT_EQ(data.numVertices(), 4u);
}

TEST_F(ObjMeshParserTests, splitsSubMeshesByUsemtl){
    ASSERT_TRUE(parse(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "usemtl firstMaterial\n"
        "f 1 2 3\n"
        "usemtl secondMaterial\n"
        "f 3 2 1\n"
    ));

    ASSERT_EQ(data.subMeshes.size(), 2u);
    ASSERT_EQ(data.subMeshes[0].materialName, "firstMaterial");
    ASSERT_EQ(data.subMeshes[1].materialName, "secondMaterial");
    ASSERT_EQ(data.subMeshes[0].indices.size(), 3u);
    ASSERT_EQ(data.subMeshes[1].indices.size(), 3u);
}

TEST_F(ObjMeshParserTests, prunesTrailingEmptySubMesh){
    ASSERT_TRUE(parse(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "f 1 2 3\n"
        "usemtl unusedMaterial\n"
    ));

    ASSERT_EQ(data.subMeshes.size(), 1u);
}

TEST_F(ObjMeshParserTests, generatesNormalsWhenMissing){
    //A counter clockwise triangle in the xy plane should face +z.
    ASSERT_TRUE(parse(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "f 1 2 3\n"
    ));

    for(size_t i = 0; i < 3; i++){
        ASSERT_EQ(normal(i), Ogre::Vector3(0, 0, 1));
    }
}

TEST_F(ObjMeshParserTests, calculatesBounds){
    ASSERT_TRUE(parse(
        "v -1 -2 -3\n"
        "v 4 0 0\n"
        "v 0 5 6\n"
        "v 100 100 100\n" //Unreferenced, so should not affect the bounds.
        "f 1 2 3\n"
    ));

    ASSERT_EQ(data.boundsMin, Ogre::Vector3(-1, -2, -3));
    ASSERT_EQ(data.boundsMax, Ogre::Vector3(4, 5, 6));
}

TEST_F(ObjMeshParserTests, ignoresCommentsAndUnknownKeywordsAndWindowsLineEndings){
    ASSERT_TRUE(parse(
        "# A comment\r\n"
        "mtllib something.mtl\r\n"
        "o someObject\r\n"
        "\r\n"
        "v 0 0 0\r\n"
        "v 1 0 0\r\n"
        "v 0 1 0\r\n"
        "s off\r\n"
        "f 1 2 3\r\n"
    ));

    ASSERT_EQ(data.numVertices(), 3u);
}

TEST_F(ObjMeshParserTests, failsWithNoFaces){
    ASSERT_FALSE(parse(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
    ));
    ASSERT_FALSE(error.empty());
}

TEST_F(ObjMeshParserTests, failsWithMalformedDefinitions){
    ASSERT_FALSE(parse("v 0 0\nf 1 1 1\n"));
    EXPECT_NE(error.find("Line 1"), std::string::npos);

    ASSERT_FALSE(parse("v 0 0 0\nvt 1\nf 1 1 1\n"));
    ASSERT_FALSE(parse("v 0 0 0\nvn 1 0\nf 1 1 1\n"));
    ASSERT_FALSE(parse("v 0 0 0\nv 1 0 0\nf 1 2\n"));
    ASSERT_FALSE(parse("v 0 0 0\nf 1 2 nonsense\n"));
}

TEST_F(ObjMeshParserTests, failsWithOutOfRangeIndices){
    ASSERT_FALSE(parse(
        "v 0 0 0\n"
        "v 1 0 0\n"
        "v 0 1 0\n"
        "f 1 2 4\n"
    ));
    EXPECT_NE(error.find("Line 4"), std::string::npos);

    //Index 0 is invalid in the obj format.
    ASSERT_FALSE(parse("v 0 0 0\nv 1 0 0\nv 0 1 0\nf 0 1 2\n"));
    //Out of range negative index.
    ASSERT_FALSE(parse("v 0 0 0\nv 1 0 0\nv 0 1 0\nf -4 1 2\n"));
    //Out of range texture coordinate.
    ASSERT_FALSE(parse("v 0 0 0\nv 1 0 0\nv 0 1 0\nvt 0 0\nf 1/2 2/1 3/1\n"));
}
