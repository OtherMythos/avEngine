#pragma once

#include "OgreString.h"
#include "OgreVector3.h"
#include "OgreQuaternion.h"
#include "OgreCommon.h"

#include <vector>

namespace Ogre{
    class SceneNode;
    class SceneManager;
    class Item;
    class ManualResourceLoader;
}

namespace AV{
    class Event;

    class OgreMeshManager{
    public:
        OgreMeshManager();
        ~OgreMeshManager();

        struct SerialisedMeshEntry{
            Ogre::SceneNode* nodePtr;
            Ogre::String meshName;
            Ogre::Vector3 pos;
            Ogre::Quaternion orientation;
        };

        typedef std::shared_ptr<Ogre::SceneNode> OgreMeshPtr;

        OgreMeshPtr createMesh(const Ogre::String& meshName);

        void setupSceneManager(Ogre::SceneManager* manager);

        /**
        Associate a file extension (including the leading dot, e.g ".obj") with a manual
        resource loader. Meshes whose name ends with that extension will be parsed by the
        given loader rather than Ogre's built in .mesh serializer. Case insensitive.

        This is the single point new mesh file formats are added; no other code needs to
        change to make a new format loadable by name.
        */
        static void registerMeshFormat(const Ogre::String& extension, Ogre::ManualResourceLoader* loader);

        /**
        If the mesh name matches a registered custom format, ensure it is registered with
        the Ogre MeshManager as a manual resource so it can subsequently be resolved by name.
        Does nothing for names with no registered format (i.e regular .mesh files). Idempotent.
        */
        static void ensureMeshRegistered(const Ogre::String& meshName, const Ogre::String& groupName);

        /**
        Create an Ogre item by mesh name, transparently handling any registered custom mesh
        format. Callers should use this rather than SceneManager::createItem directly so that
        custom formats are resolved consistently.
        */
        static Ogre::Item* createItem(Ogre::SceneManager* sceneManager, const Ogre::String& meshName,
            const Ogre::String& groupName, Ogre::SceneMemoryMgrTypes sceneType);

        bool worldEventReceiver(const Event &e);

        /**
        Iterate the meshes currently in the world and populate the serialisation data list.
        */
        void gatherMeshSerialisationData(std::vector<SerialisedMeshEntry>& serialisedMeshes);

    private:
        Ogre::SceneNode* mParentEntityNode;
        static Ogre::SceneManager* mSceneManager;

        struct MeshFormat{
            Ogre::String extension;
            Ogre::ManualResourceLoader* loader;
        };
        static std::vector<MeshFormat> mMeshFormats;

        /**
        Iterate through a scene node and destroy its items.
        This will not destroy the nodes themselves.
        */
        void _iterateAndDestroy(Ogre::SceneNode* node);

        void _repositionMeshesOriginShift(Ogre::Vector3 offset);

        static void _destroyOgreMesh(Ogre::SceneNode* sceneNode);
    };
}
