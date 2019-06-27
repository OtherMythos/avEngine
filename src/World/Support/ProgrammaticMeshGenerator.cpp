#include "ProgrammaticMeshGenerator.h"

#include <OgreMeshManager2.h>
#include <OgreMesh2.h>
#include <OgreItem.h>
#include "OgreSubMesh2.h"

namespace AV{
    Ogre::VertexArrayObject* ProgrammaticMeshGenerator::vao;

    void ProgrammaticMeshGenerator::createMesh(){
        Ogre::MeshPtr staticMesh = generateCubeMesh();

        Ogre::Item *cellItem = Ogre::Root::getSingleton().getSceneManager("Scene Manager")->createItem(staticMesh, Ogre::SCENE_DYNAMIC);
        Ogre::Root::getSingleton().getSceneManager("Scene Manager")->getRootSceneNode()->attachObject((Ogre::MovableObject*)cellItem);

        cellItem->setLocalAabb(Ogre::Aabb(Ogre::Vector3::ZERO, Ogre::Vector3(0.5, 0.5, 0.5)));


        Ogre::Item *cellItem1 = Ogre::Root::getSingleton().getSceneManager("Scene Manager")->createItem(staticMesh, Ogre::SCENE_DYNAMIC);
        Ogre::SceneNode* n = Ogre::Root::getSingleton().getSceneManager("Scene Manager")->getRootSceneNode()->createChildSceneNode();
        n->attachObject((Ogre::MovableObject*)cellItem1);
        n->setPosition(Ogre::Vector3(5, 0, 0));

        cellItem1->setLocalAabb(Ogre::Aabb(Ogre::Vector3::ZERO, Ogre::Vector3(0.5, 0.5, 0.5)));

    }


    Ogre::MeshPtr ProgrammaticMeshGenerator::generateCubeMesh(){
        static const int cubeArraySize = 12 * 3;
        static const Ogre::uint16 c_indexData[cubeArraySize]{
            0, 1, 2,
            0, 2, 3,
            4, 5, 6,
            4, 6, 7,
            8, 9, 10,
            8, 10, 11,
            12, 13, 14,
            12, 14, 15,
            16, 17, 18,
            16, 18, 19,
            20, 21, 22,
            20, 22, 23
        };

        Ogre::IndexBufferPacked *indexBuffer = createIndexBuffer(cubeArraySize, &c_indexData[0]);


        static const int cubeVerticesCount = 6 * 4 * 6;
        static const float c_originalVertices[cubeVerticesCount] = {
            1.000000,-1.000000,-1.000000, 0.000000,-1.000000,-0.000000,
            1.000000,-1.000000,1.000000, 0.000000,-1.000000,-0.000000,
            -1.000000,-1.000000,1.000000, 0.000000,-1.000000,-0.000000,
            -1.000000,-1.000000,-1.000000, 0.000000,-1.000000,-0.000000,

            1.000000,1.000000,-0.999999, 0.000000,1.000000,0.000000,
            -1.000000,1.000000,-1.000000, 0.000000,1.000000,0.000000,
            -1.000000,1.000000,1.000000, 0.000000,1.000000,0.000000,
            0.999999,1.000000,1.000001, 0.000000,1.000000,0.000000,

            1.000000,-1.000000,-1.000000, 1.000000,0.000000,0.000000,
            1.000000,1.000000,-0.999999, 1.000000,0.000000,0.000000,
            0.999999,1.000000,1.000001, 1.000000,0.000000,0.000000,
            1.000000,-1.000000,1.000000, 1.000000,0.000000,0.000000,

            1.000000,-1.000000,1.000000, -0.000000,-0.000000,1.000000,
            0.999999,1.000000,1.000001, -0.000000,-0.000000,1.000000,
            -1.000000,1.000000,1.000000, -0.000000,-0.000000,1.000000,
            -1.000000,-1.000000,1.000000, -0.000000,-0.000000,1.000000,

            -1.000000,-1.000000,1.000000, -1.000000,-0.000000,-0.000000,
            -1.000000,1.000000,1.000000, -1.000000,-0.000000,-0.000000,
            -1.000000,1.000000,-1.000000, -1.000000,-0.000000,-0.000000,
            -1.000000,-1.000000,-1.000000, -1.000000,-0.000000,-0.000000,

            1.000000,1.000000,-0.999999, 0.000000,0.000000,-1.000000,
            1.000000,-1.000000,-1.000000, 0.000000,0.000000,-1.000000,
            -1.000000,-1.000000,-1.000000, 0.000000,0.000000,-1.000000,
            -1.000000,1.000000,-1.000000, 0.000000,0.000000,-1.000000
        };

        return createStaticMesh(indexBuffer, cubeVerticesCount, &c_originalVertices[0]);
    }

    Ogre::IndexBufferPacked* ProgrammaticMeshGenerator::createIndexBuffer(int cubeArraySize, const Ogre::uint16* indexData){
        Ogre::IndexBufferPacked *indexBuffer = 0;

        Ogre::uint16 *cubeIndices = reinterpret_cast<Ogre::uint16*>( OGRE_MALLOC_SIMD(sizeof(Ogre::uint16) * cubeArraySize, Ogre::MEMCATEGORY_GEOMETRY));
        //memcpy( cubeIndices, indexData, sizeof( *indexData ) );
        memcpy( cubeIndices, indexData, sizeof(Ogre::uint16) * cubeArraySize );

        Ogre::RenderSystem *renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();
        Ogre::VaoManager *vaoManager = renderSystem->getVaoManager();

        try{
            //Actually create an index buffer and assign it to the pointer created earlier.
            //Also populate the index buffer with these values.
            //This goes, type, number of indices, Buffer type, the actual data, keep as shadow
            indexBuffer = vaoManager->createIndexBuffer(Ogre::IndexBufferPacked::IT_16BIT, cubeArraySize, Ogre::BT_IMMUTABLE, cubeIndices, true);
        }
        catch(Ogre::Exception &e){
            OGRE_FREE_SIMD( indexBuffer, Ogre::MEMCATEGORY_GEOMETRY );
            indexBuffer = 0;
            throw e;
        }

        return indexBuffer;
    }


    Ogre::MeshPtr ProgrammaticMeshGenerator::createStaticMesh(Ogre::IndexBufferPacked *indexBuffer, int arraySize, const float* vertexData){
        Ogre::RenderSystem *renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();
        Ogre::VaoManager *vaoManager = renderSystem->getVaoManager();

        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual("cube", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::SubMesh *subMesh = mesh->createSubMesh();

        Ogre::VertexElement2Vec vertexElements;
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_NORMAL));
        //vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES)


        float *cubeVertices = reinterpret_cast<float*>( OGRE_MALLOC_SIMD(sizeof(float) * arraySize, Ogre::MEMCATEGORY_GEOMETRY ) );

        memcpy(cubeVertices, vertexData, sizeof(float) * arraySize);

        Ogre::VertexBufferPacked *vertexBuffer = 0;
        try{
            vertexBuffer = vaoManager->createVertexBuffer(vertexElements, arraySize, Ogre::BT_DEFAULT, cubeVertices, true);
        }catch(Ogre::Exception &e){
            vertexBuffer = 0;
            throw e;
        }

        Ogre::VertexBufferPackedVec vertexBuffers;
        vertexBuffers.push_back(vertexBuffer);

        vao = vaoManager->createVertexArrayObject(vertexBuffers, indexBuffer, Ogre::OT_TRIANGLE_LIST);

        subMesh->mVao[Ogre::VpNormal].push_back(vao);
        subMesh->mVao[Ogre::VpShadow].push_back(vao);

        mesh->_setBounds( Ogre::Aabb( Ogre::Vector3::ZERO, Ogre::Vector3::UNIT_SCALE ), false );
        mesh->_setBoundingSphereRadius( 1.732f );

        return mesh;
    }


}
