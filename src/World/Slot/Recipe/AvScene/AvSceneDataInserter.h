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
        size_t _insertSceneData(size_t index, ParsedSceneFile* data, Ogre::SceneNode* node);
        Ogre::SceneNode* _createObject(const SceneObjectEntry& e, Ogre::SceneNode* parent);

        Ogre::SceneManager* mSceneManager;
    };
}
