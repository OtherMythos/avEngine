#include "AvSceneFileParser.h"

#include "Animation/AnimationData.h"

#include "System/Util/FileSystemHelper.h"

#include <cassert>

namespace AV{
    AVSceneFileParser::AVSceneFileParser(){

    }

    AVSceneFileParser::~AVSceneFileParser(){

    }

    bool AVSceneFileParser::loadFile(const std::string& filePath, AVSceneFileParserInterface* interface){
        tinyxml2::XMLDocument xmlDoc;
        if(!FileSystemHelper::loadXMLDocument(xmlDoc, filePath)){
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

    bool AVSceneFileParser::_readBasicValuesFromElement(tinyxml2::XMLElement* e, AVSceneFileParserInterface::ElementBasicValues& vals, AVSceneFileParserInterface* interface){
        {
            const char* nameAttrib;
            tinyxml2::XMLError queryResult = e->QueryStringAttribute("name", &nameAttrib);
            if(queryResult == tinyxml2::XML_SUCCESS){
                vals.name = nameAttrib;
            }else{
                vals.name = 0;
            }
        }

        {
            tinyxml2::XMLElement* posElem = e->FirstChildElement("position");
            if(posElem){
                bool result = _readVec3FromElement(posElem, &vals.pos, interface);
                if(!result) return false;
            }
        }

        {
            tinyxml2::XMLElement* scaleElem = e->FirstChildElement("scale");
            if(scaleElem){
                bool result = _readVec3FromElement(scaleElem, &vals.scale, interface);
                if(!result) return false;
            }
        }

        {
            tinyxml2::XMLElement* quatElem = e->FirstChildElement("orientation");
            if(quatElem){
                bool result = _readQuaternionFromElement(quatElem, &vals.orientation, interface);
                if(!result) return false;
            }
        }

        {
            int animIdxAttrib;
            tinyxml2::XMLError queryResult = e->QueryIntAttribute("animIdx", &animIdxAttrib);
            if(queryResult == tinyxml2::XML_SUCCESS){
                if(animIdxAttrib < 0 || animIdxAttrib >= MAX_ANIMATION_INFO){
                    interface->logError("Requested animIdx surpasses engine allowed entries.");
                    return false;
                }
                vals.animIdx = static_cast<uint8>(animIdxAttrib);
            }
            //Otherwise defaults to 0xFF.
        }

        return true;
    }

    bool AVSceneFileParser::_parseMeshXMLElement(AVSceneFileParserInterface* interface, tinyxml2::XMLElement* e, int parentId, int& outParentNode){
        const char* meshAttrib = 0;
        tinyxml2::XMLError queryResult = e->QueryStringAttribute("mesh", &meshAttrib);
        if(queryResult != tinyxml2::XML_SUCCESS){
            interface->logError("Error reading 'mesh' attribute from mesh tag.");
            return false;
        }

        AVSceneFileParserInterface::ElementBasicValues vals;
        if(!_readBasicValuesFromElement(e, vals, interface)){
            return false;
        }

        outParentNode = interface->createMesh(parentId, meshAttrib, vals);

        return true;
    }

    bool AVSceneFileParser::_parseUserXMLElement(int userId, AVSceneFileParserInterface* interface, tinyxml2::XMLElement* e, int parentId, int& outParentNode){
        const char* valueAttrib = 0;
        tinyxml2::XMLError queryResult = e->QueryStringAttribute("value", &valueAttrib);
        if(queryResult != tinyxml2::XML_SUCCESS){
            interface->logError("Error reading 'value' attribute from user tag.");
            return false;
        }

        AVSceneFileParserInterface::ElementBasicValues vals;
        if(!_readBasicValuesFromElement(e, vals, interface)){
            return false;
        }

        interface->createUser(userId, parentId, valueAttrib, vals);

        return true;
    }

    bool AVSceneFileParser::_parseNode(AVSceneFileParserInterface* interface, tinyxml2::XMLElement* parent, int parentId){
        for(tinyxml2::XMLElement *e = parent->FirstChildElement(); e != NULL; e = e->NextSiblingElement()){
            const char* v = e->Value();
            int parentNodeId = 0;
            if(strcmp(v, "mesh") == 0){
                bool result = _parseMeshXMLElement(interface, e, parentId, parentNodeId);
                if(!result) return false;
            }
            else if(strcmp(v, "empty") == 0){
                AVSceneFileParserInterface::ElementBasicValues vals;
                if(!_readBasicValuesFromElement(e, vals, interface)){
                    return false;
                }

                parentNodeId = interface->createEmpty(parentId, vals);
            }
            else if(strcmp(v, "user0") == 0){
                if(!_parseUserXMLElement(0, interface, e, parentId, parentNodeId)) return false;
            }else if(strcmp(v, "user1") == 0){
                if(!_parseUserXMLElement(1, interface, e, parentId, parentNodeId)) return false;
            }else if(strcmp(v, "user2") == 0){
                if(!_parseUserXMLElement(2, interface, e, parentId, parentNodeId)) return false;
            }else{
                //Skip tags as they might be things like 'position', 'scale'.
                //interface->logError("Unknown tag found");
                //return false;
            }
            bool result = _parseNode(interface, e, parentNodeId);
            if(!result) return false;
        }
        if(parentId != -1) interface->reachedEndForParent(parentId);

        return true;
    }
}
