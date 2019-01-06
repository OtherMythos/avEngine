#pragma once

#include <string>

namespace Ogre{
    class SceneManager;
    class SceneNode;
}

namespace AV{

    /**
     A chunk is a piece of map inserted into a slot in the SlotManager.
     Chunks contain resources such as meshes and physics shapes.
     */
    class Chunk{
    public:
        Chunk(const std::string &map, int chunkX, int chunkY, Ogre::SceneNode *parentNode);
        ~Chunk();

        /**
         Compare this chunk to another.

         @return
         True if this chunk has the same coordinates as passed in. False if not.
         */
        bool compare(const std::string &map, int chunkX, int chunkY);

        /**
         Activate the chunk.
         This will construct the physics shapes from the recipe and setup the entities.
         It will show the ogre meshes.

         @remarks
         The chunk must be deactive before this function can be called.
         */
        void activate();
        void deactivate();
        void move();

        int getChunkX() { return _chunkX; };
        int getChunkY() { return _chunkY; };

    private:
        int _chunkX, _chunkY;
        std::string _map;
        Ogre::SceneNode* _parentSceneNode;
        Ogre::SceneManager* _sceneManager;

        bool _activated = false;

        void _initialise();
        void _positionOgreMeshes();
    };
};
