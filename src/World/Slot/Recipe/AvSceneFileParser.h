#pragma once

#include <string>
#include "tinyxml2.h"

#include "OgreVector3.h"
#include "OgreQuaternion.h"

namespace AV{
    /**
    Provides an interface to the functions of the scene parser.
    */
    class AVSceneFileParserInterface{
    public:
        AVSceneFileParserInterface(){

        }

        virtual void log(const char* message) = 0;
        virtual void logError(const char* message) = 0;
        //Creates a node, returning the id.
        //It is the implementation's responsibility to manage the node ids.
        virtual int createEmpty(int parent, const Ogre::Vector3& pos, const Ogre::Vector3& scale, const Ogre::Quaternion& orientation) = 0;
        virtual int createMesh(int parent, const char* name, const char* mesh, const Ogre::Vector3& pos, const Ogre::Vector3& scale, const Ogre::Quaternion& orientation) = 0;
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
        static bool _parseMeshXMLElement(AVSceneFileParserInterface* interface, tinyxml2::XMLElement* e, int parentId);

        struct ElementBasicValues{
            ElementBasicValues() : pos(Ogre::Vector3::ZERO), scale(Ogre::Vector3(1, 1, 1)), orientation(Ogre::Quaternion::IDENTITY), name(0) { }
            Ogre::Vector3 pos;
            Ogre::Vector3 scale;
            Ogre::Quaternion orientation;
            const char* name;
        };
        static bool _readBasicValuesFromElement(tinyxml2::XMLElement* e, ElementBasicValues& vals, AVSceneFileParserInterface* interface);
    };
}
