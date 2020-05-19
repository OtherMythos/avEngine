#pragma once

#include "Ogre.h"

namespace AV{

    /**
    A class to create basic shapes programmatically without needing to load them from the file system.
    */
    class ProgrammaticMeshGenerator{
    public:
        static void createMesh();
        static void shutdown();

        static Ogre::VertexArrayObject* getRect2dVao() { return mRectVertexArray; }

    private:
        static Ogre::IndexBufferPacked* createIndexBuffer(int cubeArraySize, const Ogre::uint16* indexData);
        static Ogre::MeshPtr createStaticMesh(const Ogre::String& name, Ogre::IndexBufferPacked *indexBuffer, const Ogre::VertexElement2Vec& elemVec, int arraySize, const float* vertexData, Ogre::OperationType t = Ogre::OT_TRIANGLE_LIST);

        static Ogre::MeshPtr generateCubeMesh();
        static Ogre::MeshPtr generateSphereMesh();
        static Ogre::MeshPtr generateCapsuleMesh();
        static void generateLineBox();

        static void generateRect2dVao();

        static Ogre::VertexArrayObject* mRectVertexArray;

        /*struct CubeVertices{
            float px, py, pz;   //Position
            float nx, ny, nz;   //Normals

            CubeVertices() {}
            CubeVertices( float _px, float _py, float _pz,
                          float _nx, float _ny, float _nz) :
                px( _px ), py( _py ), pz( _pz ),
                nx( _nx ), ny( _ny ), nz( _nz )
            {
            }
        };*/

        struct CubeVertices{
            float px, py, pz, nx, ny, nz;
        };

    };
}
