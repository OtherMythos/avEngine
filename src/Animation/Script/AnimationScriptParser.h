#pragma once

#include "Animation/AnimationData.h"
#include "System/EnginePrerequisites.h"
#include "OgreVector3.h"
#include <string>

namespace tinyxml2{
    class XMLNode;
    class XMLElement;
}

namespace AV{

    /**
    Helper class to log the results of script parsing.
    */
    class AnimationScriptParserLogger{
    public:
        virtual void notifyError(const std::string& message) = 0;
        virtual void notifyWarning(const std::string& message) = 0;
    };

    /**
    A helper class to parse construction info from animation script files.
    */
    class AnimationScriptParser{
    public:
        AnimationScriptParser() { };
        ~AnimationScriptParser() { };

        bool parseFile(const std::string& filePath, AnimationParserOutput& outValue, AnimationScriptParserLogger* logger = 0);
        bool parseBuffer(const char* buffer, AnimationParserOutput& outValue, AnimationScriptParserLogger* logger);

    private:
        bool _initialParse(tinyxml2::XMLNode* root, AnimationScriptParserLogger* logger);
        bool _parseAnimationSequence(tinyxml2::XMLNode* node, AnimationScriptParserLogger* logger);
        bool _parseAnimationDataBlocks(tinyxml2::XMLNode* node, AnimationScriptParserLogger* logger, uint8* outValue);
        bool _parseAnimations(tinyxml2::XMLNode* node, AnimationScriptParserLogger* logger);
        bool _parseSingleAnimation(tinyxml2::XMLElement* e, AnimationScriptParserLogger* logger);
        bool _readKeyframesFromTrack(AnimationTrackType trackType, tinyxml2::XMLElement* e, size_t& currentKeyData, AnimationScriptParserLogger* logger);
        bool _getTrackUserData(AnimationTrackType t, tinyxml2::XMLElement* e, AnimationScriptParserLogger* logger, uint32& outData);

        AnimationInfoTypes _getTypeOfNode(tinyxml2::XMLElement* entry, AnimationScriptParserLogger* logger);
        AnimationTrackType _getTrackType(tinyxml2::XMLElement* e, AnimationScriptParserLogger* logger);

        bool _getVector3Value(const char* name, tinyxml2::XMLElement *entry, Keyframe& k, size_t& currentKeyData, uint32& dataValue, uint32 parseValue, KeyFrameData& d, const Ogre::Vector3& defaultVal = Ogre::Vector3::ZERO);

        void _readTransformKeyframe(tinyxml2::XMLElement *entry, size_t& currentKeyData, AnimationScriptParserLogger* logger);
        void _readPbsDiffuseKeyframe(tinyxml2::XMLElement *entry, size_t& currentKeyData, AnimationScriptParserLogger* logger);
        void _readPbsDetailMapKeyframe(tinyxml2::XMLElement *entry, size_t& currentKeyData, AnimationScriptParserLogger* logger);

        void _produceKeyframeSkipMap(const std::vector<Keyframe>& keyframes, uint32 animEnd, uint32 start, uint32 end, uint8 (&outValues)[3]) const;

        AnimationParserOutput* constructionInfo;
    };

}
