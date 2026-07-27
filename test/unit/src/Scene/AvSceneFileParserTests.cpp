#include "gtest/gtest.h"

#include <fstream>
#include <string>
#include <vector>

#include "Scene/AvScene/AvSceneFileParser.h"
#include "Scene/AvScene/AvSceneFileForDataParserInterface.h"

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
    };

    class RecordingSceneInterface : public AV::AVSceneFileParserInterface{
    public:
        std::vector<Call> calls;
        std::vector<std::string> errors;
        int idCount = 0;

        void log(const char* message) override { }
        void logError(const char* message) override { errors.push_back(message); }

        int createEmpty(int parent, const ElementBasicValues& vals) override {
            calls.push_back({Call::Type::Empty, parent, "", -1, vals});
            return ++idCount;
        }
        int createMesh(int parent, const char* mesh, const ElementBasicValues& vals) override {
            calls.push_back({Call::Type::Mesh, parent, mesh, -1, vals});
            return ++idCount;
        }
        int createUser(int userId, int parent, const char* userValue, const ElementBasicValues& vals) override {
            calls.push_back({Call::Type::User, parent, userValue, userId, vals});
            return ++idCount;
        }
        void reachedEndForParent(int parentId) override {
            calls.push_back({Call::Type::End, parentId, "", -1, ElementBasicValues()});
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
    const char* prepareSceneFile(const char* contents){
        const char* targetPath = "/tmp/avSceneParserTest.avscene";
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
    const char* file = prepareSceneFile(
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
    const char* file = prepareSceneFile(
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
    const char* file = prepareSceneFile("<!-- nothing here -->\n");

    RecordingSceneInterface interface;
    ASSERT_FALSE(AV::AVSceneFileParser::loadFile(file, &interface));
    ASSERT_FALSE(interface.errors.empty());
}

TEST_F(AvSceneFileParserTests, FailsCleanlyForMissingFile){
    RecordingSceneInterface interface;
    ASSERT_FALSE(AV::AVSceneFileParser::loadFile("/tmp/avSceneParserTestNoSuchFile.avscene", &interface));
}

//Transforms are read from child elements, and each is optional — an element that omits
//one keeps the default (zero position, unit scale, identity orientation).
TEST_F(AvSceneFileParserTests, ReadsTransformsAndDefaults){
    const char* file = prepareSceneFile(
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
    const char* file = prepareSceneFile(
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
    const char* file = prepareSceneFile(
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
    const char* file = prepareSceneFile(
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
    const char* file = prepareSceneFile(
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
