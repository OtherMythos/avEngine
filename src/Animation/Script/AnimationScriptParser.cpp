#include "AnimationScriptParser.h"

#include "tinyxml2.h"
#include "Animation/AnimationInfoBlockUtil.h"
#include "OgreStringConverter.h"

#include <set>
#include <cassert>

//TODO remove
#include <iostream>

namespace AV{

    bool AnimationScriptParser::parseFile(const std::string& filePath, AnimationParserOutput& outValue, AnimationScriptParserLogger* logger){
        tinyxml2::XMLDocument xmlDoc;

        if(xmlDoc.LoadFile(filePath.c_str()) != tinyxml2::XML_SUCCESS) {
            logger->notifyError("Error when opening the dialog script: " + filePath);
            logger->notifyError(xmlDoc.ErrorStr());
            return false;
        }

        constructionInfo = &outValue;
        return _initialParse(xmlDoc.FirstChild(), logger);
    }

    bool AnimationScriptParser::parseBuffer(const char* buffer, AnimationParserOutput& outValue, AnimationScriptParserLogger* logger){
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

        //TODO make it so I can secify multiple blocks of data in a file, and then reference them per animation.
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

        constructionInfo->infoHashes.push_back(_produceTypeHashForObjectTypes(types));
        return true;
    }

    bool AnimationScriptParser::_parseAnimations(tinyxml2::XMLNode* node, AnimationScriptParserLogger* logger){
        for(tinyxml2::XMLElement *e = node->FirstChildElement("animations"); e != NULL; e = e->NextSiblingElement("animations")){
            for(tinyxml2::XMLElement *anim = e->FirstChildElement(); anim != NULL; anim = anim->NextSiblingElement()){
                size_t startTrackSize = constructionInfo->trackDefinition.size();
                if(!_parseSingleAnimation(anim, logger)){
                    return false;
                }
                if(constructionInfo->trackDefinition.size() == startTrackSize){
                    logger->notifyWarning("No tracks were defined for an animation.");
                }
            }
        }

        return true;
    }

    bool AnimationScriptParser::_parseSingleAnimation(tinyxml2::XMLElement* e, AnimationScriptParserLogger* logger){
        const char* animName = e->Name();
        bool repeats = e->BoolAttribute("repeat", false);
        uint32 end = e->UnsignedAttribute("end", 0);

        size_t trackStart, trackEnd;
        size_t keyframeStart, keyframeEnd;
        size_t dataStart, dataEnd;

        trackStart = constructionInfo->trackDefinition.size();
        keyframeStart = constructionInfo->keyframes.size();
        dataStart = constructionInfo->data.size();

        //Parse the track information from the animation.
        for(tinyxml2::XMLElement *entry = e->FirstChildElement("t"); entry != NULL; entry = entry->NextSiblingElement("t")){
            AnimationTrackType trackType = _getTrackType(entry, logger);
            if(trackType == AnimationTrackType::None){
                logger->notifyWarning("Could not read type from track. Skipping.");
                continue;
            }

            //TODO check if trackTarget is valid for the defined list.
            uint32 trackTarget = 0;
            tinyxml2::XMLError queryResult = entry->QueryUnsignedAttribute("target", &trackTarget);
            if(queryResult != tinyxml2::XML_SUCCESS){
                logger->notifyError("Error reading 'target' from track.");
                continue;
            }

            uint32 trackKeyframeStart = constructionInfo->keyframes.size();
            _readKeyframesFromTrack(trackType, entry, logger);
            uint32 trackKeyframeEnd = constructionInfo->keyframes.size();
            if(trackKeyframeEnd == trackKeyframeStart){
                logger->notifyWarning("Provided track contains no key frames.");
            }

            uint8 values[3];
            _produceKeyframeSkipMap(constructionInfo->keyframes, end, trackKeyframeStart, trackKeyframeEnd, values);
            constructionInfo->trackDefinition.push_back({
                trackType, 0, trackKeyframeEnd - trackKeyframeStart, {values[0], values[1], values[2]}, static_cast<uint8>(trackTarget)
            });
        }

        trackEnd = constructionInfo->trackDefinition.size();
        keyframeEnd = constructionInfo->keyframes.size();
        dataEnd = constructionInfo->data.size();

        constructionInfo->animInfo.push_back(
            {animName, repeats, static_cast<uint16>(end), 0,
            trackStart, trackEnd, keyframeStart, keyframeEnd, dataStart, dataEnd}
        );

        return true;
    }

    void AnimationScriptParser::_produceKeyframeSkipMap(const std::vector<Keyframe>& keyframes, uint32 animEnd, uint32 start, uint32 end, uint8 (&outValues)[3]) const{
        memset(&outValues, 0, sizeof(outValues));
        if(start == end) return;
        uint8 quarterTime = static_cast<uint8>(float(animEnd) / 4);

        uint16 currentTime = quarterTime;
        uint32 currentKeyframe = start;
        for(uint8 i = 0; i < 3; i++){
            for(; keyframes[currentKeyframe].keyframePos < currentTime; currentKeyframe++){
                if(currentKeyframe == end) break;
            }
            uint32 setKeyframe = currentKeyframe - start;
            if(keyframes[currentKeyframe].keyframePos > currentTime){
                outValues[i] = currentKeyframe > 0 ? setKeyframe - 1 : 0;
            }
            else outValues[i] = setKeyframe;

            currentTime += quarterTime;
        }
    }

    bool AnimationScriptParser::_readKeyframesFromTrack(AnimationTrackType trackType, tinyxml2::XMLElement* e, AnimationScriptParserLogger* logger){
        size_t currentKeyData = 0;
        for(tinyxml2::XMLElement *entry = e->FirstChildElement("k"); entry != NULL; entry = entry->NextSiblingElement("k")){
            uint32 targetItem = 0;
            tinyxml2::XMLError errorValue = entry->QueryUnsignedAttribute("t", &targetItem);
            if(errorValue != tinyxml2::XML_SUCCESS) continue;

            switch(trackType){
                default:
                    assert(false); //For now
                    break;
                case AnimationTrackType::Transform:
                    _readTransformKeyframe(entry, currentKeyData, logger);
                    break;
            }
        }
        return false;
    }

    void AnimationScriptParser::_readTransformKeyframe(tinyxml2::XMLElement *entry, size_t& currentKeyData, AnimationScriptParserLogger* logger){
        uint32 dataValue = 0;
        Keyframe k;

        uint32 targetTime = 0;
        tinyxml2::XMLError errorValue = entry->QueryUnsignedAttribute("t", &targetTime);
        if(errorValue != tinyxml2::XML_SUCCESS){
            logger->notifyWarning("Could not read 't' start time value from keyframe. It will be skipped.");
            return;
        }
        k.keyframePos = static_cast<uint16>(targetTime);


        const char* position = 0;
        errorValue = entry->QueryStringAttribute("position", &position);
        if(errorValue == tinyxml2::XML_SUCCESS){
            dataValue |= KeyframeTransformTypes::Position;
            Ogre::Vector3 pos = Ogre::StringConverter::parseVector3(position, Ogre::Vector3::ZERO);
            k.a.ui = currentKeyData;
            constructionInfo->data.push_back(pos.x);
            constructionInfo->data.push_back(pos.y);
            constructionInfo->data.push_back(pos.z);
            currentKeyData += 3;
        }

        const char* scale = 0;
        errorValue = entry->QueryStringAttribute("scale", &scale);
        if(errorValue == tinyxml2::XML_SUCCESS){
            dataValue |= KeyframeTransformTypes::Scale;
            Ogre::Vector3 foundScale = Ogre::StringConverter::parseVector3(scale, Ogre::Vector3(1, 1, 1));
            k.b.ui = currentKeyData;
            constructionInfo->data.push_back(foundScale.x);
            constructionInfo->data.push_back(foundScale.y);
            constructionInfo->data.push_back(foundScale.z);
            currentKeyData += 3;
        }

        const char* orientation = 0;
        errorValue = entry->QueryStringAttribute("rot", &orientation);
        if(errorValue == tinyxml2::XML_SUCCESS){
            dataValue |= KeyframeTransformTypes::Orientation;
            Ogre::Vector3 foundRotation = Ogre::StringConverter::parseVector3(orientation, Ogre::Vector3::ZERO);
            Ogre::Matrix3 mat;
            mat.FromEulerAnglesXYZ(Ogre::Degree(foundRotation.x), Ogre::Degree(foundRotation.y), Ogre::Degree(foundRotation.z));
            Ogre::Quaternion quat(mat);
            k.c.ui = currentKeyData;
            constructionInfo->data.push_back(quat.w);
            constructionInfo->data.push_back(quat.x);
            constructionInfo->data.push_back(quat.y);
            constructionInfo->data.push_back(quat.z);
            currentKeyData += 4;
        }
        errorValue = entry->QueryStringAttribute("quat", &orientation);
        if(errorValue == tinyxml2::XML_SUCCESS){
            dataValue |= KeyframeTransformTypes::Orientation;
            Ogre::Quaternion quat = Ogre::StringConverter::parseQuaternion(orientation, Ogre::Quaternion::IDENTITY);
            k.c.ui = currentKeyData;
            constructionInfo->data.push_back(quat.w);
            constructionInfo->data.push_back(quat.x);
            constructionInfo->data.push_back(quat.y);
            constructionInfo->data.push_back(quat.z);
            currentKeyData += 4;
        }

        k.data = dataValue;
        constructionInfo->keyframes.push_back(k);
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
