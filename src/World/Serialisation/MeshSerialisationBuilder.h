#pragma once

#include <memory>

#include "World/Support/OgreMeshManager.h"

namespace AV{
    class OgreMeshManager;
    class SaveHandle;

    /**
    A class to assist the serialisation of meshes.
    */
    class MeshSerialisationBuilder{
    public:
        MeshSerialisationBuilder(std::shared_ptr<OgreMeshManager> ogreMeshManager);
        ~MeshSerialisationBuilder();

        /**
        Collect and store the mesh serialisation data for the meshes currently in the world.
        */
        void prepareSerialisationMeshData();

        /**
        Confirm that a mesh should be included in the serialisation stage, i.e an object has confirmed it is using it.

        @return
        The id of that mesh for use in the serialisation.
        */
        uint32_t confirmMesh(Ogre::SceneNode* meshNode);

        void writeMeshFile(const SaveHandle& targetSaveHandle);

    private:
        std::shared_ptr<OgreMeshManager> mOgreMeshManager;

        std::vector<OgreMeshManager::SerialisedMeshEntry> mMeshData;
        /**
        A list of the ids of claimed meshes. 0 means not claimed.
        */
        std::vector<uint32_t> mConfirmedMeshes;

        uint32_t mConfirmedMeshCounter = 0;
    };
}
