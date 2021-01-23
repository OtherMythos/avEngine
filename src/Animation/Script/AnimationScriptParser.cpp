#include "AnimationScriptParser.h"

#include "tinyxml2.h"
#include "Animation/AnimationInfoBlockUtil.h"
#include "OgreStringConverter.h"

#include <set>
#include <cassert>

//TODO remove
#include <iostream>

namespace AV{

    bool AnimationScriptParser::parseFile(const std::string& filePath, AnimationDefConstructionInfo& outValue, AnimationScriptParserLogger* logger){
        tinyxml2::XMLDocument xmlDoc;

        if(xmlDoc.LoadFile(filePath.c_str()) != tinyxml2::XML_SUCCESS) {
            logger->notifyError("Error when opening the dialog script: " + filePath);
            logger->notifyError(xmlDoc.ErrorStr());
            return false;
        }

        constructionInfo = &outValue;
        return _initialParse(xmlDoc.FirstChild(), logger);
    }

    bool AnimationScriptParser::parseBuffer(const char* buffer, AnimationDefConstructionInfo& outValue, AnimationScriptParserLogger* logger){
        tinyxml2::XMLDocument xmlDoc;

        if(xmlDoc.Parse(buffer) != tinyxml2::XML_SUCCESS) {
            logger->notifyError("Error when parsing the xml buffer.");
            logger->notifyError(xmlDoc.ErrorStr());
            return false;
        }

        constructionInfo = &outValue;
        return _initialParse(xmlDoc.FirstChild(), logger);
    }

    bool AnimationScriptParser::_initialParse(tinyxml2::XMLNode* root, AnimationScriptParserLogger* logger){
        if(!root) {
            logger->notifyError("Script file empty!");
            return false;
        }
        if(strcmp(root->Value(), "AnimationSequence") != 0){
            logger->notifyError("Animation script must contain an AnimationSequence tag.");
            return false;
        }

        return _parseAnimationSequence(root, logger);
    }

    bool AnimationScriptParser::_parseAnimationSequence(tinyxml2::XMLNode* node, AnimationScriptParserLogger* logger){
        uint8 foundValues = 0;
        if(!_parseAnimationDataBlocks(node, logger, &foundValues)) return false;
        if(foundValues == 0){
            logger->notifyError("No animation data blocks were provided, so the animation has nothing to do.");
            return false;
        }

        _parseAnimations(node, logger);

        return true;
    }

    bool AnimationScriptParser::_parseAnimationDataBlocks(tinyxml2::XMLNode* node, AnimationScriptParserLogger* logger, uint8* outValue){
        //NOTE Right now the values are only used to keep track of whether a name appeared twice.
        //Later on I might want to use it for something more complex.
        std::set<std::string> foundBlockValues;

        uint8 foundValues = 0;
        AnimationInfoTypes types[MAX_ANIMATION_INFO];
        memset(&types, 0, sizeof(types));
        for(tinyxml2::XMLElement *e = node->FirstChildElement("data"); e != NULL; e = e->NextSiblingElement("data")){

            for(tinyxml2::XMLElement *entry = e->FirstChildElement(); entry != NULL; entry = entry->NextSiblingElement()){
                const char* name = entry->Name();
                const std::string strName(name);
                if(foundBlockValues.find(strName) != foundBlockValues.end()){
                    logger->notifyWarning("A duplicate animation info '" + strName + "' was found. The previous one will be overridden.");
                    assert(foundValues > 0);
                    foundValues--;
                }
                AnimationInfoTypes type = _getTypeOfNode(entry, logger);
                if(type == ANIM_INFO_NONE){
                    logger->notifyWarning("Unable to read AnimationBlockInfo it will be skipped.");
                    continue;
                }
                types[foundValues] = type;
                foundValues++;
                foundBlockValues.insert(strName);
            }
        }

        if(foundBlockValues.size() >= MAX_ANIMATION_INFO){
            logger->notifyError("More than the max number of animation info, " + std::to_string(MAX_ANIMATION_INFO) + ", was provided.");
            return false;
        }
        assert(foundValues == foundBlockValues.size());
        *outValue = static_cast<uint8>(foundBlockValues.size());

        constructionInfo->animInfoHash = _produceTypeHashForObjectTypes(types);
        return true;
    }

    bool AnimationScriptParser::_parseAnimations(tinyxml2::XMLNode* node, AnimationScriptParserLogger* logger){
        for(tinyxml2::XMLElement *e = node->FirstChildElement("animations"); e != NULL; e = e->NextSiblingElement("animations")){
            if(!_parseSingleAnimation(e, logger)){
                return false;
            }
        }

        return true;
    }

    bool AnimationScriptParser::_parseSingleAnimation(tinyxml2::XMLElement* e, AnimationScriptParserLogger* logger){
        const char* animName = e->Name();
        bool repeats = e->BoolAttribute("repeat", false);
        uint32 end = e->UnsignedAttribute("end", 0);
        //Parse the track information from the animation.
        for(tinyxml2::XMLElement *entry = e->FirstChildElement("t"); entry != NULL; entry = entry->NextSiblingElement("t")){
            AnimationTrackType trackType = _getTrackType(e, logger);
            if(trackType == AnimationTrackType::None) continue;

            uint32 trackTarget = 0;
            tinyxml2::XMLError queryResult = e->QueryUnsignedAttribute("target", &trackTarget);
            if(queryResult != tinyxml2::XML_SUCCESS){
                logger->notifyError("Error reading target from track.");
                continue;
            }
            _readKeyframesFromTrack(trackType, entry, logger);
        }

        return true;
    }

    bool AnimationScriptParser::_readKeyframesFromTrack(AnimationTrackType trackType, tinyxml2::XMLElement* e, AnimationScriptParserLogger* logger){
        for(tinyxml2::XMLElement *entry = e->FirstChildElement("k"); entry != NULL; entry = entry->NextSiblingElement("k")){
            uint32 targetItem = 0;
            tinyxml2::XMLError errorValue = entry->QueryUnsignedAttribute("t", &targetItem);
            if(errorValue != tinyxml2::XML_SUCCESS) continue;

            switch(trackType){
                default:
                case AnimationTrackType::Transform:
                    _readTransformKeyframe(entry, logger);
                    break;
            }
        }
        return false;
    }

    void AnimationScriptParser::_readTransformKeyframe(tinyxml2::XMLElement *entry, AnimationScriptParserLogger* logger){
        int outValue = 0;

        const char* position = 0;
        tinyxml2::XMLError errorValue = entry->QueryStringAttribute("position", &position);
        if(errorValue == tinyxml2::XML_SUCCESS){
            outValue |= KeyframeTransformTypes::Position;
            Ogre::Vector3 pos = Ogre::StringConverter::parseVector3(position, Ogre::Vector3::ZERO);
        }

        const char* scale = 0;
        errorValue = entry->QueryStringAttribute("scale", &scale);
        if(errorValue == tinyxml2::XML_SUCCESS){
            outValue |= KeyframeTransformTypes::Scale;
            Ogre::Vector3 foundScale = Ogre::StringConverter::parseVector3(scale, Ogre::Vector3(1, 1, 1));
        }
    }

    AnimationTrackType AnimationScriptParser::_getTrackType(tinyxml2::XMLElement* e, AnimationScriptParserLogger* logger){
        const char* value = 0;
        tinyxml2::XMLError errorValue = e->QueryStringAttribute("type", &value);
        if(errorValue != tinyxml2::XML_SUCCESS){
            logger->notifyError("Error reading type of track.");
            return AnimationTrackType::None;
        }

        if(strcmp(value, "transform") == 0) return AnimationTrackType::Transform;
        else return AnimationTrackType::None;
    }

    AnimationInfoTypes AnimationScriptParser::_getTypeOfNode(tinyxml2::XMLElement* entry, AnimationScriptParserLogger* logger){
        const char* value = 0;
        tinyxml2::XMLError errorValue = entry->QueryStringAttribute("type", &value);
        if(errorValue != tinyxml2::XML_SUCCESS){
            logger->notifyWarning(
                errorValue == tinyxml2::XML_NO_ATTRIBUTE ? "Animation info block value does not contain a type" : "Animation info block value is not a string"
            );
            return ANIM_INFO_NONE;
        }

        if(strcmp(value, "SceneNode") == 0) return ANIM_INFO_SCENE_NODE;
        else return ANIM_INFO_NONE;
    }
}
