#pragma once

#include "System/EnginePrerequisites.h"
#include <string>
#include "tinyxml2.h"

#include "OgreVector3.h"
#include "OgreQuaternion.h"

namespace AV{
    //TODO should this be in its own header?
    /**
    Provides an interface to the functions of the scene parser.
    */
    class AVSceneFileParserInterface{
    public:
        static const uint8 NONE_ANIM_IDX = 0xFF;
        struct ElementBasicValues{
            ElementBasicValues() : pos(Ogre::Vector3::ZERO), scale(Ogre::Vector3::UNIT_SCALE), orientation(Ogre::Quaternion::IDENTITY), name(0), animIdx(NONE_ANIM_IDX) { }
            Ogre::Vector3 pos;
            Ogre::Vector3 scale;
            Ogre::Quaternion orientation;
            const char* name;
            uint8 animIdx;
        };

        AVSceneFileParserInterface(){

        }

        virtual void log(const char* message) = 0;
        virtual void logError(const char* message) = 0;
        //Creates a node, returning the id.
        //It is the implementation's responsibility to manage the node ids.
        virtual int createEmpty(int parent, const ElementBasicValues& vals) = 0;
        virtual int createMesh(int parent, const char* mesh, const ElementBasicValues& vals) = 0;
        virtual int createUser(int userId, int parent, const char* userValue, const ElementBasicValues& vals) = 0;
        virtual void reachedEndForParent(int parentId) = 0;
    };

    /**
    Parse an avSceneFile.
    Files are produced by a tool such as blender or Southsea in an xml format.
    They describe a hierarchical scene which can be imported.
    */
    class AVSceneFileParser{
    public:
        AVSceneFileParser();
        ~AVSceneFileParser();

        static bool loadFile(const std::string& filePath, AVSceneFileParserInterface* interface);

    private:
        static bool _parseNode(AVSceneFileParserInterface* interface, tinyxml2::XMLElement* e, int parentId);

        static bool _readQuaternionFromElement(tinyxml2::XMLElement* elem, Ogre::Quaternion* outQuat, AVSceneFileParserInterface* interface);
        static bool _readVec3FromElement(tinyxml2::XMLElement* elem, Ogre::Vector3* outVec, AVSceneFileParserInterface* interface);
        static bool _parseMeshXMLElement(AVSceneFileParserInterface* interface, tinyxml2::XMLElement* e, int parentId, int& outParentNode);
        static bool _parseUserXMLElement(int userId, AVSceneFileParserInterface* interface, tinyxml2::XMLElement* e, int parentId, int& outParentNode);

        static bool _readBasicValuesFromElement(tinyxml2::XMLElement* e, AVSceneFileParserInterface::ElementBasicValues& vals, AVSceneFileParserInterface* interface);
    };
}
