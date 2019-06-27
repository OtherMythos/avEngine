#pragma once

#include "Ogre.h"

namespace AV{

    /**
    A class to create basic shapes programmatically without needing to load them from the file system.
    */
    class ProgrammaticMeshGenerator{
    public:
        static void createMesh();

    private:
        static Ogre::IndexBufferPacked* createIndexBuffer(int cubeArraySize, const Ogre::uint16* indexData);
        static Ogre::MeshPtr createStaticMesh(Ogre::IndexBufferPacked *indexBuffer, int arraySize, const float* vertexData);

        static Ogre::MeshPtr generateCubeMesh();
        static Ogre::MeshPtr generateCapsuleMesh();

        static Ogre::VertexArrayObject *vao;

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
