#include "ProgrammaticMeshGenerator.h"

#include <OgreMeshManager2.h>
#include <OgreMesh2.h>
#include <OgreItem.h>
#include "OgreSubMesh2.h"

#include <math.h>

namespace AV{
    void ProgrammaticMeshGenerator::createMesh(){
        generateSphereMesh();
        generateCubeMesh();
    }

    Ogre::MeshPtr ProgrammaticMeshGenerator::generateSphereMesh(){
        static const int stackCount = 10;
        static const int sectorCount = 20;
        static const int radius = 1;

        static const float M_PI = 3.14;


        std::vector<int> indices;
        // generate CCW index list of sphere triangles
        for(int i = 0; i < stackCount; ++i){
            int k1 = i * (sectorCount + 1);     // beginning of current stack
            int k2 = k1 + sectorCount + 1;      // beginning of next stack

            for(int j = 0; j < sectorCount; ++j, ++k1, ++k2){
                if(i != 0){
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }

                if(i != (stackCount-1)){
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }

        //Windows (msvc) doesn't like compiling variable length arrays on the stack, so I have to do it as pointers.
        Ogre::uint16* c_indexData = new Ogre::uint16[indices.size()];

        for(int i = 0; i < indices.size(); i++){
            c_indexData[i] = indices[i];
        }

        Ogre::IndexBufferPacked *indexBuffer = createIndexBuffer(indices.size(), c_indexData);

        std::vector<float> verts;

        float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
        float s, t;                                     // vertex texCoord

        const float sectorStep = 2 * M_PI / sectorCount;
        const float stackStep = M_PI / stackCount;

        for(int i = 0; i <= stackCount; ++i){
            float stackAngle = M_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            float xy = radius * cosf(stackAngle);             // r * cos(u)
            float z = radius * sinf(stackAngle);              // r * sin(u)

            // add (sectorCount+1) vertices per stack
            // the first and last vertices have same position and normal, but different tex coords
            for(int j = 0; j <= sectorCount; ++j){
                float sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                // vertex position (x, y, z)
                float x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
                float y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
                verts.push_back(x);
                verts.push_back(y);
                verts.push_back(z);

                // normalized vertex normal (nx, ny, nz)
                nx = x * lengthInv;
                ny = y * lengthInv;
                nz = z * lengthInv;
                verts.push_back(nx);
                verts.push_back(ny);
                verts.push_back(nz);

                /*// vertex tex coord (s, t) range between [0, 1]
                s = (float)j / sectorCount;
                t = (float)i / stackCount;
                texCoords.push_back(s);
                texCoords.push_back(t);*/
            }
        }

        const int cubeVerticesCount = verts.size();
        float* c_originalVertices = new float[cubeVerticesCount];

        for(int i = 0; i < verts.size(); i++){
            c_originalVertices[i] = verts[i];
        }

        Ogre::MeshPtr retMesh = createStaticMesh("sphere", indexBuffer, cubeVerticesCount, c_originalVertices);

        delete[] c_indexData;
        delete[] c_originalVertices;

        return retMesh;

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

            1.000000,1.000000,-1.000000, 0.000000,1.000000,0.000000,
            -1.000000,1.000000,-1.000000, 0.000000,1.000000,0.000000,
            -1.000000,1.000000,1.000000, 0.000000,1.000000,0.000000,
            1.000000,1.000000,1.000001, 0.000000,1.000000,0.000000,

            1.000000,-1.000000,-1.000000, 1.000000,0.000000,0.000000,
            1.000000,1.000000,-1.000000, 1.000000,0.000000,0.000000,
            1.000000,1.000000,1.000001, 1.000000,0.000000,0.000000,
            1.000000,-1.000000,1.000000, 1.000000,0.000000,0.000000,

            1.000000,-1.000000,1.000000, -0.000000,-0.000000,1.000000,
            1.000000,1.000000,1.000001, -0.000000,-0.000000,1.000000,
            -1.000000,1.000000,1.000000, -0.000000,-0.000000,1.000000,
            -1.000000,-1.000000,1.000000, -0.000000,-0.000000,1.000000,

            -1.000000,-1.000000,1.000000, -1.000000,-0.000000,-0.000000,
            -1.000000,1.000000,1.000000, -1.000000,-0.000000,-0.000000,
            -1.000000,1.000000,-1.000000, -1.000000,-0.000000,-0.000000,
            -1.000000,-1.000000,-1.000000, -1.000000,-0.000000,-0.000000,

            1.000000,1.000000,-1.000000, 0.000000,0.000000,-1.000000,
            1.000000,-1.000000,-1.000000, 0.000000,0.000000,-1.000000,
            -1.000000,-1.000000,-1.000000, 0.000000,0.000000,-1.000000,
            -1.000000,1.000000,-1.000000, 0.000000,0.000000,-1.000000
        };

        return createStaticMesh("cube", indexBuffer, cubeVerticesCount, &c_originalVertices[0]);
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


    Ogre::MeshPtr ProgrammaticMeshGenerator::createStaticMesh(const Ogre::String& name, Ogre::IndexBufferPacked *indexBuffer, int arraySize, const float* vertexData){
        Ogre::RenderSystem *renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();
        Ogre::VaoManager *vaoManager = renderSystem->getVaoManager();

        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual(name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
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

        Ogre::VertexArrayObject* vao = vaoManager->createVertexArrayObject(vertexBuffers, indexBuffer, Ogre::OT_TRIANGLE_LIST);

        subMesh->mVao[Ogre::VpNormal].push_back(vao);
        subMesh->mVao[Ogre::VpShadow].push_back(vao);

        mesh->_setBounds( Ogre::Aabb( Ogre::Vector3::ZERO, Ogre::Vector3::UNIT_SCALE ), false );
        mesh->_setBoundingSphereRadius( 1.732f );

        return mesh;
    }


}
