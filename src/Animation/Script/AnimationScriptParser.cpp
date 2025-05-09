#include "AnimationScriptParser.h"

#include "tinyxml2.h"
#include "Animation/AnimationInfoBlockUtil.h"
#include "OgreStringConverter.h"
#include "System/Util/FileSystemHelper.h"

#include <set>
#include <cassert>

//TODO remove
#include <iostream>

namespace AV{

    bool AnimationScriptParser::parseFile(const std::string& filePath, AnimationParserOutput& outValue, AnimationScriptParserLogger* logger){
        tinyxml2::XMLDocument xmlDoc;
        bool result = FileSystemHelper::loadXMLDocument(xmlDoc, filePath);

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

        for(tinyxml2::XMLElement *e = node->FirstChildElement("data"); e != NULL; e = e->NextSiblingElement("data")){
            uint8 foundValues = 0;
            AnimationInfoTypes types[MAX_ANIMATION_INFO];
            memset(&types, 0, sizeof(types));

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
            if(foundBlockValues.size() >= MAX_ANIMATION_INFO){
                logger->notifyError("More than the max number of animation info, " + std::to_string(MAX_ANIMATION_INFO) + ", was provided.");
                return false;
            }
            assert(foundValues == foundBlockValues.size());
            *outValue = static_cast<uint8>(foundBlockValues.size());

            constructionInfo->infoHashes.push_back(_produceTypeHashForObjectTypes(types));
            foundBlockValues.clear();
        }

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
        uint32 targetData = e->UnsignedAttribute("data", 0);
        if(targetData >= 255){
            logger->notifyWarning("Animation contains invalid 'data' id. 0 will be used.");
            targetData = 0;
        }
        if(targetData >= constructionInfo->infoHashes.size()){
            logger->notifyError("Animation references an invalid data id.");
            return false;
        }

        size_t trackStart, trackEnd;
        size_t keyframeStart, keyframeEnd;
        size_t dataStart, dataEnd;

        trackStart = constructionInfo->trackDefinition.size();
        keyframeStart = constructionInfo->keyframes.size();
        dataStart = constructionInfo->data.size();

        //Different animations need to start at 0, but each track should increment from there.
        uint32 trackStartCounter = 0;
        size_t currentTrackDataCount = 0;

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

            uint32 trackUserData = 0;
            bool success = _getTrackUserData(trackType, entry, logger, trackUserData);
            if(!success) continue;

            uint32 trackKeyframeStart = constructionInfo->keyframes.size();
            _readKeyframesFromTrack(trackType, entry, currentTrackDataCount, logger);
            uint32 trackKeyframeEnd = constructionInfo->keyframes.size();
            uint32 trackKeyframeDiff = trackKeyframeEnd - trackKeyframeStart;
            if(trackKeyframeDiff == 0){
                logger->notifyWarning("Provided track contains no key frames.");
            }

            uint8 values[3];
            _produceKeyframeSkipMap(constructionInfo->keyframes, end, trackKeyframeStart, trackKeyframeEnd, values);
            constructionInfo->trackDefinition.push_back({
                trackType, trackStartCounter, trackStartCounter+trackKeyframeDiff, {values[0], values[1], values[2]}, static_cast<uint8>(trackTarget), trackUserData
            });
            trackStartCounter += trackKeyframeDiff;
        }

        trackEnd = constructionInfo->trackDefinition.size();
        keyframeEnd = constructionInfo->keyframes.size();
        dataEnd = constructionInfo->data.size();

        constructionInfo->animInfo.push_back(
            {animName, repeats, static_cast<uint16>(end), static_cast<uint8>(targetData),
            trackStart, trackEnd, keyframeStart, keyframeEnd, dataStart, dataEnd}
        );

        return true;
    }

    bool AnimationScriptParser::_getTrackUserData(AnimationTrackType t, tinyxml2::XMLElement* e, AnimationScriptParserLogger* logger, uint32& outData){
        switch(t){
            case AnimationTrackType::PBS_DETAIL_MAP:{
                uint32 detailMapTarget = 0;
                tinyxml2::XMLError errorValue = e->QueryUnsignedAttribute("detailMap", &detailMapTarget);
                if(errorValue != tinyxml2::XML_SUCCESS){
                    logger->notifyWarning("Could not read 'target' from detail map track. It will be skipped.");
                    return false;
                }
                if(detailMapTarget >= 255){
                    logger->notifyWarning("Detail map target must be in range 0 and 255.");
                    return false;
                }
                assert(detailMapTarget < 255);
                outData = detailMapTarget;
                break;
            }
            default:
                outData = 0;
                break;
        }

        //Read the easing data, for instance sin, cos, etc.
        _getFrameTypeValue(e, outData);

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

    bool AnimationScriptParser::_readKeyframesFromTrack(AnimationTrackType trackType, tinyxml2::XMLElement* e, size_t& currentKeyData, AnimationScriptParserLogger* logger){
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
                case AnimationTrackType::PBS_DIFFUSE:
                    _readPbsDiffuseKeyframe(entry, currentKeyData, logger);
                    break;
                case AnimationTrackType::PBS_DETAIL_MAP:
                    _readPbsDetailMapKeyframe(entry, currentKeyData, logger);
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

        _getVector3Value("position", entry, k, currentKeyData, dataValue, KeyframeTransformTypes::Position, k.a);
        _getVector3Value("scale", entry, k, currentKeyData, dataValue, KeyframeTransformTypes::Scale, k.b, Ogre::Vector3(1, 1, 1));
        bool hasRot = _getVector3Value("rot", entry, k, currentKeyData, dataValue, KeyframeTransformTypes::Rotation, k.c);

        const char* orientation = 0;
        errorValue = entry->QueryStringAttribute("quat", &orientation);
        if(errorValue == tinyxml2::XML_SUCCESS){
            if(hasRot){
                logger->notifyWarning("Keyframe defines both 'rot' and 'quat'. 'quat' will be ignored.");
            }else{
                dataValue |= KeyframeTransformTypes::Orientation;
                Ogre::Quaternion quat = Ogre::StringConverter::parseQuaternion(orientation, Ogre::Quaternion::IDENTITY);
                k.c.ui = currentKeyData;
                constructionInfo->data.push_back(quat.w);
                constructionInfo->data.push_back(quat.x);
                constructionInfo->data.push_back(quat.y);
                constructionInfo->data.push_back(quat.z);
                currentKeyData += 4;
            }
        }

        k.data = dataValue;
        constructionInfo->keyframes.push_back(k);
    }

    void AnimationScriptParser::_readPbsDiffuseKeyframe(tinyxml2::XMLElement *entry, size_t& currentKeyData, AnimationScriptParserLogger* logger){
        //TODO remove duplication
        uint32 dataValue = 0;
        Keyframe k;

        uint32 targetTime = 0;
        tinyxml2::XMLError errorValue = entry->QueryUnsignedAttribute("t", &targetTime);
        if(errorValue != tinyxml2::XML_SUCCESS){
            logger->notifyWarning("Could not read 't' start time value from keyframe. It will be skipped.");
            return;
        }
        k.keyframePos = static_cast<uint16>(targetTime);

        _getVector3Value("diffuseColour", entry, k, currentKeyData, dataValue, KeyframePbsSetTypes::DiffuseSet, k.a);

        k.data = dataValue;
        constructionInfo->keyframes.push_back(k);
    }

    void AnimationScriptParser::_readPbsDetailMapKeyframe(tinyxml2::XMLElement *entry, size_t& currentKeyData, AnimationScriptParserLogger* logger){
        //TODO remove duplication
        uint32 dataValue = 0;
        Keyframe k;

        uint32 targetTime = 0;
        tinyxml2::XMLError errorValue = entry->QueryUnsignedAttribute("t", &targetTime);
        if(errorValue != tinyxml2::XML_SUCCESS){
            logger->notifyWarning("Could not read 't' start time value from keyframe. It will be skipped.");
            return;
        }
        k.keyframePos = static_cast<uint16>(targetTime);

        uint32 targetAIdx = currentKeyData;
        const char* offset;
        errorValue = entry->QueryStringAttribute("offset", &offset);
        if(errorValue == tinyxml2::XML_SUCCESS){
            dataValue |= KeyframePbsDetailMapTypes::OffsetSet;
            Ogre::Vector2 vec = Ogre::StringConverter::parseVector2(offset, Ogre::Vector2::ZERO);
            constructionInfo->data.push_back(vec.x);
            constructionInfo->data.push_back(vec.y);
            currentKeyData += 2;
        }
        const char* scale;
        errorValue = entry->QueryStringAttribute("scale", &scale);
        if(errorValue == tinyxml2::XML_SUCCESS){
            dataValue |= KeyframePbsDetailMapTypes::ScaleSet;
            Ogre::Vector2 vec = Ogre::StringConverter::parseVector2(scale, Ogre::Vector2(1, 1));
            constructionInfo->data.push_back(vec.x);
            constructionInfo->data.push_back(vec.y);
            currentKeyData += 2;
        }

        float weight = 0;
        errorValue = entry->QueryFloatAttribute("weight", &weight);
        if(errorValue == tinyxml2::XML_SUCCESS){
            dataValue |= KeyframePbsDetailMapTypes::WeightSet;
            k.b.f = weight;
        }

        float normalWeight = 0;
        errorValue = entry->QueryFloatAttribute("normWeight", &normalWeight);
        if(errorValue == tinyxml2::XML_SUCCESS){
            dataValue |= KeyframePbsDetailMapTypes::NormalWeightSet;
            k.c.f = normalWeight;
        }

        k.a.ui = targetAIdx;
        k.data = dataValue;
        constructionInfo->keyframes.push_back(k);
    }

    bool AnimationScriptParser::_getFrameTypeValue(tinyxml2::XMLElement *entry, uint32& dataValue){
        const char* value = 0;
        tinyxml2::XMLError errorValue = entry->QueryStringAttribute("frameEasing", &value);
        if(errorValue == tinyxml2::XML_SUCCESS){
            uint32 easingValue = FrameEasingType::FrameEasingLinear;
            if(strcmp(value, "easeInSine") == 0) easingValue = FrameEasingType::FrameEasingEaseInSine;
            else if(strcmp(value, "easeOutSine") == 0) easingValue = FrameEasingType::FrameEasingEaseOutSine;
            else if(strcmp(value, "easeInOutSine") == 0) easingValue = FrameEasingType::FrameEasingEaseInOutSine;
            else if(strcmp(value, "easeInCubic") == 0) easingValue = FrameEasingType::FrameEasingEaseInCubic;
            else if(strcmp(value, "easeOutCubic") == 0) easingValue = FrameEasingType::FrameEasingEaseOutCubic;
            else if(strcmp(value, "easeInOutCubic") == 0) easingValue = FrameEasingType::FrameEasingEaseInOutCubic;
            dataValue |= easingValue << ANIM_EASING_SHIFT_BITS;
            return true;
        }
        return false;
    }

    bool AnimationScriptParser::_getVector3Value(const char* name, tinyxml2::XMLElement *entry, Keyframe& k, size_t& currentKeyData, uint32& dataValue, uint32 parseValue, KeyFrameData& d, const Ogre::Vector3& defaultVecVal){
        const char* value = 0;
        tinyxml2::XMLError errorValue = entry->QueryStringAttribute(name, &value);
        if(errorValue == tinyxml2::XML_SUCCESS){
            dataValue |= parseValue;
            Ogre::Vector3 pos = Ogre::StringConverter::parseVector3(value, defaultVecVal);
            d.ui = currentKeyData;
            constructionInfo->data.push_back(pos.x);
            constructionInfo->data.push_back(pos.y);
            constructionInfo->data.push_back(pos.z);
            currentKeyData += 3;
            return true;
        }
        return false;
    }

    AnimationTrackType AnimationScriptParser::_getTrackType(tinyxml2::XMLElement* e, AnimationScriptParserLogger* logger){
        const char* value = 0;
        tinyxml2::XMLError errorValue = e->QueryStringAttribute("type", &value);
        if(errorValue != tinyxml2::XML_SUCCESS){
            logger->notifyError("Error reading type of track.");
            return AnimationTrackType::None;
        }

        if(strcmp(value, "transform") == 0) return AnimationTrackType::Transform;
        else if(strcmp(value, "pbsDiffuse") == 0) return AnimationTrackType::PBS_DIFFUSE;
        else if(strcmp(value, "pbsDetailMap") == 0) return AnimationTrackType::PBS_DETAIL_MAP;
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
        else if(strcmp(value, "pbsDatablock") == 0) return ANIM_INFO_PBS_DATABLOCK;
        else return ANIM_INFO_NONE;
    }
}
