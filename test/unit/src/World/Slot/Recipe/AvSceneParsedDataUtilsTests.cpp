#include "gtest/gtest.h"

#include "World/Slot/Recipe/AvScene/AvSceneParsedData.h"
#include "World/Slot/Recipe/AvScene/AvSceneParsedDataUtils.h"


/**
Tests for AvSceneParsedDataUtils::computeDerivedTransform.

A parsed scene stores each object's transform relative to its parent, exactly as the scene file
described it. Anything asking a parsed scene "where is this thing actually going to end up" needs
those transforms composed down the parent chain, and the composition has to match
Ogre::Node::_updateFromParent or the answer will disagree with the scene once it's inserted.

The scenes here are built by hand rather than parsed, so the composition is tested on its own.
Expected values are literals computed from the Ogre formula off to the side — deriving them with
a second implementation in the test would only restate the code under test.
*/
class AvSceneParsedDataUtilsTests : public ::testing::Test {
public:
    //Append an object parented to parentIdx (-1 for the root), returning its index.
    static uint32_t addObject(AV::ParsedSceneFile& file, int parentIdx,
        const Ogre::Vector3& pos, const Ogre::Vector3& scale, const Ogre::Quaternion& orientation){

        const uint32_t idx = static_cast<uint32_t>(file.data.size());
        file.objects.push_back({AV::SceneObjectType::Empty});
        file.data.push_back({0, pos, scale, orientation, 0xFF, -1, -1, parentIdx, AV::SceneObjectType::Empty});
        file.childIndices.push_back({});
        if(parentIdx < 0) file.rootIndices.push_back(idx);
        else file.childIndices[parentIdx].push_back(idx);

        return idx;
    }

    static void expectVec3(const Ogre::Vector3& actual, float x, float y, float z){
        ASSERT_NEAR(actual.x, x, 0.0001f);
        ASSERT_NEAR(actual.y, y, 0.0001f);
        ASSERT_NEAR(actual.z, z, 0.0001f);
    }

    static void expectQuat(const Ogre::Quaternion& actual, float w, float x, float y, float z){
        ASSERT_NEAR(actual.w, w, 0.0001f);
        ASSERT_NEAR(actual.x, x, 0.0001f);
        ASSERT_NEAR(actual.y, y, 0.0001f);
        ASSERT_NEAR(actual.z, z, 0.0001f);
    }

    //90 degrees about the given axis.
    static Ogre::Quaternion quarterTurn(const Ogre::Vector3& axis){
        Ogre::Quaternion q;
        q.FromAngleAxis(Ogre::Radian(Ogre::Math::HALF_PI), axis);
        return q;
    }
};

//A root object has nothing above it, so its derived transform is just its own.
TEST_F(AvSceneParsedDataUtilsTests, RootTransformIsItsLocalTransform){
    AV::ParsedSceneFile file;
    const Ogre::Quaternion orientation = quarterTurn(Ogre::Vector3::UNIT_Y);
    uint32_t root = addObject(file, -1, Ogre::Vector3(5, -2, 8), Ogre::Vector3(2, 2, 2), orientation);

    Ogre::Vector3 pos, scale;
    Ogre::Quaternion orient;
    AV::AvSceneParsedDataUtils::computeDerivedTransform(file, root, &pos, &scale, &orient);

    expectVec3(pos, 5, -2, 8);
    expectVec3(scale, 2, 2, 2);
    expectQuat(orient, orientation.w, orientation.x, orientation.y, orientation.z);
}

//The interesting case: the parent both rotates and scales non-uniformly, so a child's position
//has to be scaled first, then rotated, then offset. Getting that order wrong still produces
//plausible looking numbers, which is why the expected values are exact.
TEST_F(AvSceneParsedDataUtilsTests, ChildOfRotatedScaledParentComposesLikeOgre){
    AV::ParsedSceneFile file;
    uint32_t parent = addObject(file, -1,
        Ogre::Vector3(10, 0, 0), Ogre::Vector3(2, 3, 4), quarterTurn(Ogre::Vector3::UNIT_Y));
    uint32_t child = addObject(file, static_cast<int>(parent),
        Ogre::Vector3(1, 2, 3), Ogre::Vector3(0.5f, 0.5f, 0.5f), quarterTurn(Ogre::Vector3::UNIT_X));

    Ogre::Vector3 pos, scale;
    Ogre::Quaternion orient;
    AV::AvSceneParsedDataUtils::computeDerivedTransform(file, child, &pos, &scale, &orient);

    //(2,3,4)*(1,2,3) = (2,6,12), turned a quarter about Y to (12,6,-2), offset by (10,0,0).
    expectVec3(pos, 22, 6, -2);
    expectVec3(scale, 1, 1.5f, 2);
    expectQuat(orient, 0.5f, 0.5f, 0.5f, -0.5f);
}

//Three deep, so the accumulated values from the first step have to be fed into the second
//rather than each level being composed against the root.
TEST_F(AvSceneParsedDataUtilsTests, ThreeDeepChainAccumulates){
    AV::ParsedSceneFile file;
    uint32_t parent = addObject(file, -1,
        Ogre::Vector3(10, 0, 0), Ogre::Vector3(2, 3, 4), quarterTurn(Ogre::Vector3::UNIT_Y));
    uint32_t child = addObject(file, static_cast<int>(parent),
        Ogre::Vector3(1, 2, 3), Ogre::Vector3(0.5f, 0.5f, 0.5f), quarterTurn(Ogre::Vector3::UNIT_X));
    uint32_t grandChild = addObject(file, static_cast<int>(child),
        Ogre::Vector3(0, 1, 0), Ogre::Vector3(2, 2, 2), Ogre::Quaternion::IDENTITY);

    Ogre::Vector3 pos, scale;
    Ogre::Quaternion orient;
    AV::AvSceneParsedDataUtils::computeDerivedTransform(file, grandChild, &pos, &scale, &orient);

    //(1,1.5,2)*(0,1,0) = (0,1.5,0), turned by the accumulated orientation to (1.5,0,0).
    expectVec3(pos, 23.5f, 6, -2);
    expectVec3(scale, 2, 3, 4);
    //An identity local orientation leaves the accumulated one alone.
    expectQuat(orient, 0.5f, 0.5f, 0.5f, -0.5f);
}

//Siblings must not see each other's transforms, only the shared parent's.
TEST_F(AvSceneParsedDataUtilsTests, SiblingsAreIndependent){
    AV::ParsedSceneFile file;
    uint32_t parent = addObject(file, -1,
        Ogre::Vector3(0, 10, 0), Ogre::Vector3(2, 2, 2), Ogre::Quaternion::IDENTITY);
    uint32_t first = addObject(file, static_cast<int>(parent),
        Ogre::Vector3(1, 0, 0), Ogre::Vector3::UNIT_SCALE, Ogre::Quaternion::IDENTITY);
    uint32_t second = addObject(file, static_cast<int>(parent),
        Ogre::Vector3(0, 0, 1), Ogre::Vector3::UNIT_SCALE, Ogre::Quaternion::IDENTITY);

    Ogre::Vector3 firstPos, secondPos;
    AV::AvSceneParsedDataUtils::computeDerivedTransform(file, first, &firstPos, 0, 0);
    AV::AvSceneParsedDataUtils::computeDerivedTransform(file, second, &secondPos, 0, 0);

    expectVec3(firstPos, 2, 10, 0);
    expectVec3(secondPos, 0, 10, 2);
}

//Callers only ever want one of the three, so each output has to be optional.
TEST_F(AvSceneParsedDataUtilsTests, OutputsAreOptional){
    AV::ParsedSceneFile file;
    uint32_t parent = addObject(file, -1,
        Ogre::Vector3(0, 10, 0), Ogre::Vector3(2, 2, 2), Ogre::Quaternion::IDENTITY);
    uint32_t child = addObject(file, static_cast<int>(parent),
        Ogre::Vector3(1, 0, 0), Ogre::Vector3(3, 3, 3), Ogre::Quaternion::IDENTITY);

    Ogre::Vector3 scale;
    AV::AvSceneParsedDataUtils::computeDerivedTransform(file, child, 0, &scale, 0);

    expectVec3(scale, 6, 6, 6);
}

//Tag lookup is the whole point of the tags map, so it has to find a tagged object and say so
//plainly when there isn't one, rather than returning an index that happens to be in range.
TEST_F(AvSceneParsedDataUtilsTests, FindsNodeIndexByTag){
    AV::ParsedSceneFile file;
    addObject(file, -1, Ogre::Vector3::ZERO, Ogre::Vector3::UNIT_SCALE, Ogre::Quaternion::IDENTITY);
    uint32_t tagged = addObject(file, -1, Ogre::Vector3::ZERO, Ogre::Vector3::UNIT_SCALE, Ogre::Quaternion::IDENTITY);
    file.strings.push_back("spawn");
    file.data[tagged].tag = 0;
    file.tags["spawn"] = tagged;

    ASSERT_EQ(AV::AvSceneParsedDataUtils::findNodeIndexByTag(file, "spawn"), static_cast<int>(tagged));
    ASSERT_EQ(AV::AvSceneParsedDataUtils::findNodeIndexByTag(file, "notATag"), -1);
    //Index 0 is a real object, so a null tag must not be answered with it.
    ASSERT_EQ(AV::AvSceneParsedDataUtils::findNodeIndexByTag(file, 0), -1);
}
