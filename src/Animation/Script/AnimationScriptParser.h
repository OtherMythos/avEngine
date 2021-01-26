#pragma once

#include "Animation/AnimationData.h"
#include "System/EnginePrerequisites.h"
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
        bool _readKeyframesFromTrack(AnimationTrackType trackType, tinyxml2::XMLElement* e, AnimationScriptParserLogger* logger);

        AnimationInfoTypes _getTypeOfNode(tinyxml2::XMLElement* entry, AnimationScriptParserLogger* logger);
        AnimationTrackType _getTrackType(tinyxml2::XMLElement* e, AnimationScriptParserLogger* logger);

        void _readTransformKeyframe(tinyxml2::XMLElement *entry, size_t& currentKeyData, AnimationScriptParserLogger* logger);

        AnimationParserOutput* constructionInfo;
    };

}
