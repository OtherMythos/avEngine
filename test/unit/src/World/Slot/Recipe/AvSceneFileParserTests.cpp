#include "gtest/gtest.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "World/Slot/Recipe/AvScene/AvSceneFileParser.h"
#include "World/Slot/Recipe/AvScene/AvSceneFileForDataParserInterface.h"

/**
Tests for the avScene xml parser (AVSceneFileParser).

The parser only talks to an AVSceneFileParserInterface, so it can be driven with a
recording implementation and needs no Ogre scene at all. RecordingSceneInterface below
captures the call sequence, which is what actually matters: WHICH element became a node,
what parent id it was given, and where the parser reported the end of a parent, since
that ordering is what the inserters replay to build the hierarchy.
*/
class AvSceneFileParserTests : public ::testing::Test {
public:
    struct Call{
        enum class Type{ Empty, Mesh, User, End } type;
        int parent;
        std::string name;      //mesh path for Mesh, user value for User
        int userId;
        AV::AVSceneFileParserInterface::ElementBasicValues vals;
        //vals.name and vals.tag point into the xml document, which is destroyed by the time a
        //test reads them back, so the tag is copied out while the call is being made.
        bool hadTag;
        std::string tag;
    };

    class RecordingSceneInterface : public AV::AVSceneFileParserInterface{
    public:
        std::vector<Call> calls;
        std::vector<std::string> errors;
        int idCount = 0;

        void log(const char* message) override { }
        void logError(const char* message) override { errors.push_back(message); }

        int createEmpty(int parent, const ElementBasicValues& vals) override {
            calls.push_back({Call::Type::Empty, parent, "", -1, vals, vals.tag != 0, vals.tag ? vals.tag : ""});
            return ++idCount;
        }
        int createMesh(int parent, const char* mesh, const ElementBasicValues& vals) override {
            calls.push_back({Call::Type::Mesh, parent, mesh, -1, vals, vals.tag != 0, vals.tag ? vals.tag : ""});
            return ++idCount;
        }
        int createUser(int userId, int parent, const char* userValue, const ElementBasicValues& vals) override {
            calls.push_back({Call::Type::User, parent, userValue, userId, vals, vals.tag != 0, vals.tag ? vals.tag : ""});
            return ++idCount;
        }
        void reachedEndForParent(int parentId) override {
            calls.push_back({Call::Type::End, parentId, "", -1, ElementBasicValues(), false, ""});
        }

        //The nth node-creating call, skipping the End markers.
        const Call& node(size_t idx) const {
            size_t seen = 0;
            for(const Call& c : calls){
                if(c.type == Call::Type::End) continue;
                if(seen == idx) return c;
                seen++;
            }
            throw std::runtime_error("no such node call");
        }
        size_t nodeCount() const {
            size_t n = 0;
            for(const Call& c : calls) if(c.type != Call::Type::End) n++;
            return n;
        }
    };

    //Writes contents to a path the parser can load.
    std::string prepareSceneFile(const char* contents){
        std::string targetPath = (std::filesystem::temp_directory_path() / "avSceneParserTest.avscene").string();
        std::ofstream outfile;
        outfile.open(targetPath);
        outfile << contents << std::endl;
        outfile.close();

        return targetPath;
    }
};

//An xml declaration is the first NODE of the document but not the first ELEMENT, and
//every exporter writes one. Taking FirstChild()->ToElement() gave null here and the
//parser dereferenced it.
TEST_F(AvSceneFileParserTests, ParsesFileWithXmlDeclaration){
    std::string file = prepareSceneFile(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<scene>\n"
        "    <mesh mesh=\"cube.mesh\"/>\n"
        "</scene>\n"
    );

    RecordingSceneInterface interface;
    ASSERT_TRUE(AV::AVSceneFileParser::loadFile(file, &interface));

    ASSERT_EQ(interface.nodeCount(), 1);
    ASSERT_EQ(interface.node(0).type, Call::Type::Mesh);
    ASSERT_EQ(interface.node(0).name, "cube.mesh");
}

//As above for a leading comment, which is also a node and also not an element.
TEST_F(AvSceneFileParserTests, ParsesFileWithLeadingComment){
    std::string file = prepareSceneFile(
        "<!-- a room, exported by hand -->\n"
        "<scene>\n"
        "    <mesh mesh=\"cube.mesh\"/>\n"
        "</scene>\n"
    );

    RecordingSceneInterface interface;
    ASSERT_TRUE(AV::AVSceneFileParser::loadFile(file, &interface));

    ASSERT_EQ(interface.nodeCount(), 1);
    ASSERT_EQ(interface.node(0).name, "cube.mesh");
}

//A file with no element at all has to be reported, not crashed on.
TEST_F(AvSceneFileParserTests, FailsCleanlyWithNoRootElement){
    std::string file = prepareSceneFile("<!-- nothing here -->\n");

    RecordingSceneInterface interface;
    ASSERT_FALSE(AV::AVSceneFileParser::loadFile(file, &interface));
    ASSERT_FALSE(interface.errors.empty());
}

TEST_F(AvSceneFileParserTests, FailsCleanlyForMissingFile){
    std::string missingPath = (std::filesystem::temp_directory_path() / "avSceneParserTestNoSuchFile.avscene").string();

    RecordingSceneInterface interface;
    ASSERT_FALSE(AV::AVSceneFileParser::loadFile(missingPath, &interface));
}

//Transforms are read from child elements, and each is optional — an element that omits
//one keeps the default (zero position, unit scale, identity orientation).
TEST_F(AvSceneFileParserTests, ReadsTransformsAndDefaults){
    std::string file = prepareSceneFile(
        "<scene>\n"
        "    <mesh mesh=\"floor.mesh\">\n"
        "        <position x=\"1.5\" y=\"-2\" z=\"3\"/>\n"
        "        <scale x=\"10\" y=\"0.2\" z=\"8\"/>\n"
        "        <orientation x=\"0\" y=\"1\" z=\"0\" w=\"0\"/>\n"
        "    </mesh>\n"
        "    <mesh mesh=\"bare.mesh\"/>\n"
        "</scene>\n"
    );

    RecordingSceneInterface interface;
    ASSERT_TRUE(AV::AVSceneFileParser::loadFile(file, &interface));

    ASSERT_EQ(interface.nodeCount(), 2);
    ASSERT_EQ(interface.node(0).vals.pos, Ogre::Vector3(1.5, -2, 3));
    ASSERT_EQ(interface.node(0).vals.scale, Ogre::Vector3(10, 0.2, 8));
    ASSERT_EQ(interface.node(0).vals.orientation, Ogre::Quaternion(0, 0, 1, 0));

    ASSERT_EQ(interface.node(1).vals.pos, Ogre::Vector3::ZERO);
    ASSERT_EQ(interface.node(1).vals.scale, Ogre::Vector3::UNIT_SCALE);
    ASSERT_EQ(interface.node(1).vals.orientation, Ogre::Quaternion::IDENTITY);
}

//Nesting: a child element is created with its parent's id, and the parser reports the
//end of that parent once its children are done. -1 is the file root.
TEST_F(AvSceneFileParserTests, ParsesHierarchy){
    std::string file = prepareSceneFile(
        "<scene>\n"
        "    <empty>\n"
        "        <position x=\"0\" y=\"1\" z=\"0\"/>\n"
        "        <mesh mesh=\"a.mesh\"/>\n"
        "        <mesh mesh=\"b.mesh\"/>\n"
        "    </empty>\n"
        "    <mesh mesh=\"c.mesh\"/>\n"
        "</scene>\n"
    );

    RecordingSceneInterface interface;
    ASSERT_TRUE(AV::AVSceneFileParser::loadFile(file, &interface));

    ASSERT_EQ(interface.nodeCount(), 4);
    //The group, then its two children carrying its id, then a sibling back at the root.
    ASSERT_EQ(interface.node(0).type, Call::Type::Empty);
    ASSERT_EQ(interface.node(0).parent, -1);
    ASSERT_EQ(interface.node(1).name, "a.mesh");
    ASSERT_EQ(interface.node(1).parent, 1);
    ASSERT_EQ(interface.node(2).name, "b.mesh");
    ASSERT_EQ(interface.node(2).parent, 1);
    ASSERT_EQ(interface.node(3).name, "c.mesh");
    ASSERT_EQ(interface.node(3).parent, -1);

    //The group's end must be reported after its children and before nothing else
    //claims their parent id.
    bool sawEndForGroup = false;
    for(const Call& c : interface.calls){
        if(c.type == Call::Type::End && c.parent == 1) sawEndForGroup = true;
    }
    ASSERT_TRUE(sawEndForGroup);
}

TEST_F(AvSceneFileParserTests, ParsesUserMarkers){
    std::string file = prepareSceneFile(
        "<scene>\n"
        "    <user2 value=\"spawnPoint\">\n"
        "        <position x=\"4\" y=\"0\" z=\"5\"/>\n"
        "    </user2>\n"
        "</scene>\n"
    );

    RecordingSceneInterface interface;
    ASSERT_TRUE(AV::AVSceneFileParser::loadFile(file, &interface));

    ASSERT_EQ(interface.nodeCount(), 1);
    ASSERT_EQ(interface.node(0).type, Call::Type::User);
    ASSERT_EQ(interface.node(0).userId, 2);
    ASSERT_EQ(interface.node(0).name, "spawnPoint");
    ASSERT_EQ(interface.node(0).vals.pos, Ogre::Vector3(4, 0, 5));
}

//A tag the parser doesn't know creates no node, so its children are handed parent id 0.
//An interface must be able to read that as "the file root" — see the 1 based ids in
//SimpleSceneFileParserInterface, which would otherwise index a node that isn't there.
TEST_F(AvSceneFileParserTests, UnknownWrapperTagLeavesChildrenAtRootParentId){
    std::string file = prepareSceneFile(
        "<scene>\n"
        "    <group>\n"
        "        <mesh mesh=\"inWrapper.mesh\"/>\n"
        "    </group>\n"
        "</scene>\n"
    );

    RecordingSceneInterface interface;
    ASSERT_TRUE(AV::AVSceneFileParser::loadFile(file, &interface));

    ASSERT_EQ(interface.nodeCount(), 1);
    ASSERT_EQ(interface.node(0).name, "inWrapper.mesh");
    ASSERT_EQ(interface.node(0).parent, 0);
}

//The data interface is what insertParsedSceneFile replays, so its output has to come
//out balanced: one entry per object plus the Child/Term markers that describe descent.
TEST_F(AvSceneFileParserTests, DataInterfaceProducesBalancedTree){
    std::string file = prepareSceneFile(
        "<?xml version=\"1.0\"?>\n"
        "<scene>\n"
        "    <empty>\n"
        "        <mesh mesh=\"a.mesh\"/>\n"
        "    </empty>\n"
        "    <mesh mesh=\"b.mesh\"/>\n"
        "</scene>\n"
    );

    AV::ParsedSceneFile parsed;
    AV::AvSceneFileForDataParserInterface interface(&parsed);
    ASSERT_TRUE(AV::AVSceneFileParser::loadFile(file, &interface));

    //Empty, Child, Mesh(a), Term, Mesh(b) — three objects and one balanced descent.
    ASSERT_EQ(parsed.data.size(), 3);
    size_t children = 0, terms = 0;
    for(const AV::SceneObjectEntry& e : parsed.objects){
        if(e.type == AV::SceneObjectType::Child) children++;
        if(e.type == AV::SceneObjectType::Term) terms++;
    }
    ASSERT_EQ(children, 1);
    ASSERT_EQ(terms, children);
    ASSERT_EQ(parsed.objects.size(), parsed.data.size() + children + terms);
}

//The parsed data records each object's parent so a scene can be queried without being
//inserted. Parent is an index into data, which the interface derives from the parser's
//creation-order ids, so a nested tree is where it can go wrong.
TEST_F(AvSceneFileParserTests, DataInterfaceRecordsParentAndChildIndices){
    std::string file = prepareSceneFile(
        "<scene>\n"
        "    <empty name=\"outer\">\n"
        "        <mesh name=\"inner\" mesh=\"a.mesh\">\n"
        "            <empty name=\"deep\"/>\n"
        "        </mesh>\n"
        "        <mesh name=\"sibling\" mesh=\"b.mesh\"/>\n"
        "    </empty>\n"
        "    <mesh name=\"secondRoot\" mesh=\"c.mesh\"/>\n"
        "</scene>\n"
    );

    AV::ParsedSceneFile parsed;
    AV::AvSceneFileForDataParserInterface interface(&parsed);
    ASSERT_TRUE(AV::AVSceneFileParser::loadFile(file, &interface));

    //outer(0) -> inner(1) -> deep(2), outer -> sibling(3), secondRoot(4)
    ASSERT_EQ(parsed.data.size(), 5);
    ASSERT_EQ(parsed.data[0].parent, -1);
    ASSERT_EQ(parsed.data[1].parent, 0);
    ASSERT_EQ(parsed.data[2].parent, 1);
    ASSERT_EQ(parsed.data[3].parent, 0);
    ASSERT_EQ(parsed.data[4].parent, -1);

    ASSERT_EQ(parsed.rootIndices.size(), 2);
    ASSERT_EQ(parsed.rootIndices[0], 0);
    ASSERT_EQ(parsed.rootIndices[1], 4);

    ASSERT_EQ(parsed.childIndices.size(), parsed.data.size());
    ASSERT_EQ(parsed.childIndices[0].size(), 2);
    ASSERT_EQ(parsed.childIndices[0][0], 1);
    ASSERT_EQ(parsed.childIndices[0][1], 3);
    ASSERT_EQ(parsed.childIndices[1].size(), 1);
    ASSERT_EQ(parsed.childIndices[1][0], 2);
    ASSERT_TRUE(parsed.childIndices[2].empty());
    ASSERT_TRUE(parsed.childIndices[4].empty());
}

//Each data entry carries its own type so a node object can report what it describes
//without walking the objects list to find its marker.
TEST_F(AvSceneFileParserTests, DataInterfaceRecordsObjectType){
    std::string file = prepareSceneFile(
        "<scene>\n"
        "    <empty/>\n"
        "    <mesh mesh=\"a.mesh\"/>\n"
        "    <user2 value=\"spawn\"/>\n"
        "</scene>\n"
    );

    AV::ParsedSceneFile parsed;
    AV::AvSceneFileForDataParserInterface interface(&parsed);
    ASSERT_TRUE(AV::AVSceneFileParser::loadFile(file, &interface));

    ASSERT_EQ(parsed.data.size(), 3);
    ASSERT_EQ(parsed.data[0].type, AV::SceneObjectType::Empty);
    ASSERT_EQ(parsed.data[1].type, AV::SceneObjectType::Mesh);
    ASSERT_EQ(parsed.data[2].type, AV::SceneObjectType::User2);

    //The type in data has to agree with the marker the inserter replays.
    ASSERT_EQ(parsed.objects[0].type, AV::SceneObjectType::Empty);
    ASSERT_EQ(parsed.objects[1].type, AV::SceneObjectType::Mesh);
    ASSERT_EQ(parsed.objects[2].type, AV::SceneObjectType::User2);
}

//The name and mesh path both live in the same strings vector, so an object has to point at
//the right one of the two entries it added.
TEST_F(AvSceneFileParserTests, DataInterfaceKeepsNameAndMeshStringsDistinct){
    std::string file = prepareSceneFile(
        "<scene>\n"
        "    <mesh name=\"crate\" mesh=\"crate.mesh\"/>\n"
        "    <mesh mesh=\"unnamed.mesh\"/>\n"
        "</scene>\n"
    );

    AV::ParsedSceneFile parsed;
    AV::AvSceneFileForDataParserInterface interface(&parsed);
    ASSERT_TRUE(AV::AVSceneFileParser::loadFile(file, &interface));

    ASSERT_EQ(parsed.data.size(), 2);
    ASSERT_EQ(parsed.strings[parsed.data[0].name], "crate");
    ASSERT_EQ(parsed.strings[parsed.data[0].idx], "crate.mesh");
    ASSERT_EQ(parsed.data[1].name, -1);
    ASSERT_EQ(parsed.strings[parsed.data[1].idx], "unnamed.mesh");
}

//The tag attribute has to reach every kind of node, not just meshes, since an empty is the
//natural thing to tag as a marker.
TEST_F(AvSceneFileParserTests, ReadsTagAttributeFromEveryNodeType){
    std::string file = prepareSceneFile(
        "<scene>\n"
        "    <empty tag=\"spawn\"/>\n"
        "    <mesh mesh=\"a.mesh\" tag=\"crate\"/>\n"
        "    <user1 value=\"trigger\" tag=\"door\"/>\n"
        "    <mesh mesh=\"b.mesh\"/>\n"
        "</scene>\n"
    );

    RecordingSceneInterface interface;
    ASSERT_TRUE(AV::AVSceneFileParser::loadFile(file, &interface));

    ASSERT_EQ(interface.nodeCount(), 4);
    ASSERT_TRUE(interface.node(0).hadTag);
    ASSERT_EQ(interface.node(0).tag, "spawn");
    ASSERT_TRUE(interface.node(1).hadTag);
    ASSERT_EQ(interface.node(1).tag, "crate");
    ASSERT_TRUE(interface.node(2).hadTag);
    ASSERT_EQ(interface.node(2).tag, "door");
    //An untagged node must report no tag rather than whatever the previous element had.
    ASSERT_FALSE(interface.node(3).hadTag);
}

//The tags map is what a lookup goes through, so it has to point at the right object and
//an untagged object has to be distinguishable from one tagged with the first string.
TEST_F(AvSceneFileParserTests, DataInterfaceBuildsTagLookup){
    std::string file = prepareSceneFile(
        "<scene>\n"
        "    <empty name=\"outer\">\n"
        "        <mesh name=\"crate\" mesh=\"a.mesh\" tag=\"theCrate\"/>\n"
        "        <empty tag=\"spawn\"/>\n"
        "    </empty>\n"
        "</scene>\n"
    );

    AV::ParsedSceneFile parsed;
    AV::AvSceneFileForDataParserInterface interface(&parsed);
    ASSERT_TRUE(AV::AVSceneFileParser::loadFile(file, &interface));
    ASSERT_FALSE(interface.hasError());

    ASSERT_EQ(parsed.data.size(), 3);
    ASSERT_EQ(parsed.tags.size(), 2);
    ASSERT_EQ(parsed.tags.at("theCrate"), 1);
    ASSERT_EQ(parsed.tags.at("spawn"), 2);

    //An untagged object holds -1, not an index into strings.
    ASSERT_EQ(parsed.data[0].tag, -1);
    ASSERT_EQ(parsed.strings[parsed.data[1].tag], "theCrate");
    ASSERT_EQ(parsed.strings[parsed.data[2].tag], "spawn");
    //The name is interned separately and must not have been overwritten by the tag.
    ASSERT_EQ(parsed.strings[parsed.data[1].name], "crate");
}

//A tag claimed twice would make a lookup return whichever object registered first, silently.
//The parser has no error channel for this, so the interface has to carry it out itself.
TEST_F(AvSceneFileParserTests, DuplicateTagIsReportedAsAnError){
    std::string file = prepareSceneFile(
        "<scene>\n"
        "    <mesh mesh=\"a.mesh\" tag=\"spawn\"/>\n"
        "    <mesh mesh=\"b.mesh\" tag=\"spawn\"/>\n"
        "</scene>\n"
    );

    AV::ParsedSceneFile parsed;
    AV::AvSceneFileForDataParserInterface interface(&parsed);
    //The parser itself sees nothing wrong, which is exactly why hasError has to be checked.
    ASSERT_TRUE(AV::AVSceneFileParser::loadFile(file, &interface));

    ASSERT_TRUE(interface.hasError());
    ASSERT_NE(interface.getError().find("spawn"), std::string::npos);
    //The first claim stands, so the map never ends up pointing at the second object.
    ASSERT_EQ(parsed.tags.at("spawn"), 0);
}

//Two objects may share a name, so the uniqueness check must be looking at tags alone.
TEST_F(AvSceneFileParserTests, DuplicateNameIsNotAnError){
    std::string file = prepareSceneFile(
        "<scene>\n"
        "    <mesh name=\"Cube\" mesh=\"a.mesh\"/>\n"
        "    <mesh name=\"Cube\" mesh=\"b.mesh\"/>\n"
        "</scene>\n"
    );

    AV::ParsedSceneFile parsed;
    AV::AvSceneFileForDataParserInterface interface(&parsed);
    ASSERT_TRUE(AV::AVSceneFileParser::loadFile(file, &interface));

    ASSERT_FALSE(interface.hasError());
}
