#include "AvSceneFileParser.h"

#include <cassert>

namespace AV{
    AVSceneFileParser::AVSceneFileParser(){

    }

    AVSceneFileParser::~AVSceneFileParser(){

    }

    bool AVSceneFileParser::loadFile(const std::string& filePath, AVSceneFileParserInterface* interface){
        tinyxml2::XMLDocument xmlDoc;

        if(xmlDoc.LoadFile(filePath.c_str()) != tinyxml2::XML_SUCCESS) {
            const std::string logError = "Error when opening the scene file: " + filePath;
            interface->log(logError.c_str());
            interface->log(xmlDoc.ErrorStr());
            return false;
        }

        tinyxml2::XMLNode *root = xmlDoc.FirstChild();
        tinyxml2::XMLElement* elem = root->ToElement();
        int parentNode = -1;
        bool result = _parseNode(interface, elem, parentNode);
        if(!result){
            interface->logError("Error parsing scene file");
            return false;
        }

        return true;
    }

    bool AVSceneFileParser::_readVec3FromElement(tinyxml2::XMLElement* elem, Ogre::Vector3* outVec, AVSceneFileParserInterface* interface){
        float x, y, z;
        tinyxml2::XMLError queryResult = elem->QueryFloatAttribute("x", &x);
        if(queryResult != tinyxml2::XML_SUCCESS){
            interface->logError("Error reading 'x'");
            return false;
        }
        queryResult = elem->QueryFloatAttribute("y", &y);
        if(queryResult != tinyxml2::XML_SUCCESS){
            interface->logError("Error reading 'y'");
            return false;
        }
        queryResult = elem->QueryFloatAttribute("z", &z);
        if(queryResult != tinyxml2::XML_SUCCESS){
            interface->logError("Error reading 'z'");
            return false;
        }

        outVec->x = x;
        outVec->y = y;
        outVec->z = z;

        return true;
    }

    bool AVSceneFileParser::_readQuaternionFromElement(tinyxml2::XMLElement* elem, Ogre::Quaternion* outQuat, AVSceneFileParserInterface* interface){
        float x, y, z, w;
        tinyxml2::XMLError queryResult = elem->QueryFloatAttribute("x", &x);
        if(queryResult != tinyxml2::XML_SUCCESS){
            interface->logError("Error reading 'x'");
            return false;
        }
        queryResult = elem->QueryFloatAttribute("y", &y);
        if(queryResult != tinyxml2::XML_SUCCESS){
            interface->logError("Error reading 'y'");
            return false;
        }
        queryResult = elem->QueryFloatAttribute("z", &z);
        if(queryResult != tinyxml2::XML_SUCCESS){
            interface->logError("Error reading 'z'");
            return false;
        }
        queryResult = elem->QueryFloatAttribute("w", &w);
        if(queryResult != tinyxml2::XML_SUCCESS){
            interface->logError("Error reading 'w'");
            return false;
        }

        outQuat->x = x;
        outQuat->y = y;
        outQuat->z = z;
        outQuat->w = w;

        return true;
    }


    bool AVSceneFileParser::_parseMeshXMLElement(AVSceneFileParserInterface* interface, tinyxml2::XMLElement* e, int parentId){
        const char* meshAttrib = 0;
        const char* nameAttrib = 0;
        tinyxml2::XMLError queryResult = e->QueryStringAttribute("mesh", &meshAttrib);
        if(queryResult != tinyxml2::XML_SUCCESS){
            interface->logError("Error reading 'mesh' attribute from mesh tag.");
            return false;
        }
        queryResult = e->QueryStringAttribute("name", &nameAttrib);
        if(queryResult != tinyxml2::XML_SUCCESS){
            interface->logError("Error reading 'name' attribute from mesh tag.");
            return false;
        }

        Ogre::Vector3 targetPos;
        { //Read position
            tinyxml2::XMLElement* posElem = e->FirstChildElement("position");
            if(!posElem){
                interface->logError("Mesh object does not contain a position.");
                return false;
            }

            bool result = _readVec3FromElement(posElem, &targetPos, interface);
            if(!result) return false;
        }

        Ogre::Vector3 targetScale;
        { //Read scale
            tinyxml2::XMLElement* scaleElem = e->FirstChildElement("scale");
            if(!scaleElem){
                interface->logError("Mesh object does not contain a scale.");
                return false;
            }

            bool result = _readVec3FromElement(scaleElem, &targetScale, interface);
            if(!result) return false;
        }

        Ogre::Quaternion targetOrientation;
        { //Read orientation
            tinyxml2::XMLElement* quatElem = e->FirstChildElement("orientation");
            if(!quatElem){
                interface->logError("Mesh object does not contain an orientation.");
                return false;
            }

            bool result = _readQuaternionFromElement(quatElem, &targetOrientation, interface);
            if(!result) return false;
        }

        interface->createMesh(parentId, nameAttrib, meshAttrib, targetPos, targetScale, targetOrientation);

        return true;
    }

    bool AVSceneFileParser::_parseNode(AVSceneFileParserInterface* interface, tinyxml2::XMLElement* parent, int parentId){
        for(tinyxml2::XMLElement *e = parent->FirstChildElement(); e != NULL; e = e->NextSiblingElement()){
            const char* v = e->Value();
            if(strcmp(v, "mesh") == 0){
                bool result = _parseMeshXMLElement(interface, e, parentId);
                if(!result) return false;
            }
            else if(strcmp(v, "node") == 0){
                int parentNodeId = interface->createNode(parentId);
                bool result = _parseNode(interface, e, parentNodeId);
                if(!result) return false;
            }
            else{
                interface->logError("Unknown tag found");
                return false;
            }
        }

        return true;
    }
}
