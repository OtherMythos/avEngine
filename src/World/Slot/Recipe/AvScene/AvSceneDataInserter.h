#pragma once

#include "AvSceneParsedData.h"

namespace Ogre{
    class SceneNode;
    class SceneManager;
}

namespace AV{

    /**
    A class to insert a parsed avScene file into a scene node.
    */
    class AVSceneDataInserter{
    public:
        AVSceneDataInserter(Ogre::SceneManager* sceneManager);
        ~AVSceneDataInserter();

        /**
        Insert a compiled scene into the specified node.
        */
        void insertSceneData(ParsedSceneFile* data, Ogre::SceneNode* node);

    private:
        size_t _insertSceneData(size_t index, size_t& createdObjectCount, ParsedSceneFile* data, Ogre::SceneNode* node);
        Ogre::SceneNode* _createObject(const SceneObjectEntry& e, const SceneObjectData& d, const std::vector<Ogre::String>& strings, Ogre::SceneNode* parent);

        Ogre::SceneManager* mSceneManager;
    };
}
