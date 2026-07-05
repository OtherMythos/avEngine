#include "ObjMeshLoader.h"

#include "ObjMeshParser.h"

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2.h"
#include "OgreSubMesh2.h"
#include "OgreResourceGroupManager.h"
#include "OgreString.h"
#include "Vao/OgreVaoManager.h"

namespace AV{

    ObjMeshLoader& ObjMeshLoader::getLoader(){
        static ObjMeshLoader loader;
        return loader;
    }

    void ObjMeshLoader::loadResource(Ogre::Resource* resource){
        Ogre::Mesh* mesh = static_cast<Ogre::Mesh*>(resource);

        Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(
            mesh->getName(), mesh->getGroup());

        ObjMeshData data;
        std::string error;
        if(!ObjMeshParser::parse(stream, data, error)){
            OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS,
                "Failed to parse obj file '" + mesh->getName() + "'. " + error,
                "ObjMeshLoader::loadResource");
        }

        Ogre::VaoManager* vaoManager = Ogre::Root::getSingletonPtr()->getRenderSystem()->getVaoManager();

        const size_t numVertices = data.numVertices();
        static const Ogre::VertexElement2Vec vertexElements = {
            Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION),
            Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_NORMAL),
            Ogre::VertexElement2(Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES)
        };

        const size_t vertexBytes = data.vertices.size() * sizeof(float);
        float* vertexData = reinterpret_cast<float*>(
            OGRE_MALLOC_SIMD(vertexBytes, Ogre::MEMCATEGORY_GEOMETRY));
        memcpy(vertexData, data.vertices.data(), vertexBytes);

        Ogre::VertexBufferPacked* vertexBuffer = 0;
        try{
            //keepAsShadow true, so the buffer contents remain readable and the vao manager owns the memory.
            vertexBuffer = vaoManager->createVertexBuffer(
                vertexElements, numVertices, Ogre::BT_IMMUTABLE, vertexData, true);
        }catch(Ogre::Exception&){
            OGRE_FREE_SIMD(vertexData, Ogre::MEMCATEGORY_GEOMETRY);
            throw;
        }

        const Ogre::IndexType indexType =
            numVertices > 0xFFFF ? Ogre::IndexType::IT_32BIT : Ogre::IndexType::IT_16BIT;

        for(const ObjMeshData::SubMesh& parsedSubMesh : data.subMeshes){
            const size_t numIndices = parsedSubMesh.indices.size();

            void* indexData = 0;
            if(indexType == Ogre::IndexType::IT_32BIT){
                indexData = OGRE_MALLOC_SIMD(numIndices * sizeof(Ogre::uint32), Ogre::MEMCATEGORY_GEOMETRY);
                memcpy(indexData, parsedSubMesh.indices.data(), numIndices * sizeof(Ogre::uint32));
            }else{
                Ogre::uint16* shortIndices = reinterpret_cast<Ogre::uint16*>(
                    OGRE_MALLOC_SIMD(numIndices * sizeof(Ogre::uint16), Ogre::MEMCATEGORY_GEOMETRY));
                for(size_t i = 0; i < numIndices; i++){
                    shortIndices[i] = static_cast<Ogre::uint16>(parsedSubMesh.indices[i]);
                }
                indexData = shortIndices;
            }

            Ogre::IndexBufferPacked* indexBuffer = 0;
            try{
                indexBuffer = vaoManager->createIndexBuffer(
                    indexType, numIndices, Ogre::BT_IMMUTABLE, indexData, true);
            }catch(Ogre::Exception&){
                OGRE_FREE_SIMD(indexData, Ogre::MEMCATEGORY_GEOMETRY);
                throw;
            }

            Ogre::VertexBufferPackedVec vertexBuffers;
            vertexBuffers.push_back(vertexBuffer);
            Ogre::VertexArrayObject* vao =
                vaoManager->createVertexArrayObject(vertexBuffers, indexBuffer, Ogre::OT_TRIANGLE_LIST);

            Ogre::SubMesh* subMesh = mesh->createSubMesh();
            subMesh->mVao[Ogre::VpNormal].push_back(vao);
            subMesh->mVao[Ogre::VpShadow].push_back(vao);
            if(!parsedSubMesh.materialName.empty()){
                subMesh->setMaterialName(parsedSubMesh.materialName);
            }
        }

        const Ogre::Aabb bounds = Ogre::Aabb::newFromExtents(data.boundsMin, data.boundsMax);
        mesh->_setBounds(bounds, true);
        mesh->_setBoundingSphereRadius(bounds.getRadius());
    }
}
