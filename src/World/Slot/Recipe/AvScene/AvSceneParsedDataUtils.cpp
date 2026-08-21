#include "AvSceneParsedDataUtils.h"

#include <cassert>

namespace AV{

    void AvSceneParsedDataUtils::computeDerivedTransform(const ParsedSceneFile& file, uint32 index,
        Ogre::Vector3* outPos, Ogre::Vector3* outScale, Ogre::Quaternion* outOrientation){

        assert(index < file.data.size());

        //The chain has to be applied root first, but it can only be walked leaf first.
        std::vector<uint32> chain;
        int current = static_cast<int>(index);
        while(current >= 0){
            chain.push_back(static_cast<uint32>(current));
            const SceneObjectData& d = file.data[current];
            //Guard against a malformed parent index rather than walking off the end.
            current = (d.parent >= 0 && static_cast<size_t>(d.parent) < file.data.size() && d.parent != current) ? d.parent : -1;
        }

        Ogre::Vector3 derivedPos(Ogre::Vector3::ZERO);
        Ogre::Vector3 derivedScale(Ogre::Vector3::UNIT_SCALE);
        Ogre::Quaternion derivedOrientation(Ogre::Quaternion::IDENTITY);

        for(auto it = chain.rbegin(); it != chain.rend(); ++it){
            const SceneObjectData& d = file.data[*it];

            //Position is transformed by the parent's accumulated values, so it has to be
            //calculated before those values absorb this object.
            derivedPos = derivedOrientation * (derivedScale * d.pos) + derivedPos;
            derivedOrientation = derivedOrientation * d.orientation;
            derivedScale = derivedScale * d.scale;
        }

        if(outPos) *outPos = derivedPos;
        if(outScale) *outScale = derivedScale;
        if(outOrientation) *outOrientation = derivedOrientation;
    }

    int AvSceneParsedDataUtils::findNodeIndexByTag(const ParsedSceneFile& file, const char* tag){
        if(!tag) return -1;

        auto it = file.tags.find(tag);
        if(it == file.tags.end()) return -1;

        return static_cast<int>(it->second);
    }

}
