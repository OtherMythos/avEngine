#include "MeshSerialisationBuilder.h"

#include "Serialisation/SaveHandle.h"

#include "SerialisationHelper.h"

namespace AV{
    MeshSerialisationBuilder::MeshSerialisationBuilder(std::shared_ptr<OgreMeshManager> ogreMeshManager)
        : mOgreMeshManager(ogreMeshManager) {

    }

    MeshSerialisationBuilder::~MeshSerialisationBuilder(){

    }

    void MeshSerialisationBuilder::prepareSerialisationMeshData(){
        mConfirmedMeshCounter = 0;
        mMeshData.clear();
        mConfirmedMeshes.clear();

        mOgreMeshManager->gatherMeshSerialisationData(mMeshData);

        //0 means not claimed. Insert enough of these to match the size of the mesh data list.
        //mConfirmedMeshes.insert(mConfirmedMeshes.begin(), mMeshData.size(), 0);
    }

    uint32_t MeshSerialisationBuilder::confirmMesh(Ogre::SceneNode* meshNode){
        for(size_t i = 0; i < mMeshData.size(); i++){
            if(mMeshData[i].nodePtr == meshNode){
                uint32_t meshId = mConfirmedMeshCounter + 1;
                //mConfirmedMeshes[i] = meshId;
                mConfirmedMeshes.push_back(i);

                mConfirmedMeshCounter++;
                return meshId;
            }
        }

        //The search should always find the mesh, otherwise the data is wrong.
        assert(false);
        //To shutup cppcheck.
        return 0;
    }

    void MeshSerialisationBuilder::writeMeshFile(const SaveHandle& targetSaveHandle){
        const Ogre::String targetFile = targetSaveHandle.determineMeshSerialisedFile();


        std::ofstream file;
        file.open(targetFile);

        for(uint32_t claimId : mConfirmedMeshes){
            const OgreMeshManager::SerialisedMeshEntry& entry = mMeshData[claimId];

            file << entry.meshName << std::endl;
            file << SER_OGER_VECTOR3(entry.pos) << std::endl;
            file << SER_OGER_QUAT(entry.orientation) << std::endl;
            file << "--";
        }

        file.close();
    }
}
