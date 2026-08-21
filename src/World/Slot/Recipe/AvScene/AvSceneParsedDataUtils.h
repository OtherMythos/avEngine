#pragma once

#include "AvSceneParsedData.h"

namespace AV{
    /**
    Helpers for querying a ParsedSceneFile without instantiating it into a scene.
    */
    class AvSceneParsedDataUtils{
    public:
        AvSceneParsedDataUtils() = delete;

        /**
        Determine the world space transform of an object by walking its parent chain.

        The accumulation matches Ogre::Node::_updateFromParent exactly, so the values returned
        here are the ones the object would end up with once the scene is inserted, assuming the
        node it's inserted under is at the origin.

        @param index Index into file.data. Must be in range.
        */
        static void computeDerivedTransform(const ParsedSceneFile& file, uint32 index,
            Ogre::Vector3* outPos, Ogre::Vector3* outScale, Ogre::Quaternion* outOrientation);

        /**
        Find the object carrying a tag.

        @returns The index into file.data, or -1 if no object in this scene has that tag.
        */
        static int findNodeIndexByTag(const ParsedSceneFile& file, const char* tag);
    };
}
