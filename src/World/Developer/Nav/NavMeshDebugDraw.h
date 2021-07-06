#ifdef DEBUGGING_TOOLS

#pragma once

#include "DebugDraw.h"
#include "DetourDebugDraw.h"
#include "Ogre.h"

#include <vector>

namespace AV{

    struct SouthseaDebugDraw : public duDebugDraw{
    public:
        SouthseaDebugDraw();
        virtual ~SouthseaDebugDraw();
        virtual void depthMask(bool state);
        virtual void texture(bool state);
        virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f);
        virtual void vertex(const float* pos, unsigned int color);
        virtual void vertex(const float x, const float y, const float z, unsigned int color);
        virtual void vertex(const float* pos, unsigned int color, const float* uv);
        virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v);
        virtual void end();

        Ogre::MeshPtr endMesh();
    private:
        void _vertex(float x, float y, float z, unsigned int color);
        void _createFromMesh();

        void createStaticMesh(Ogre::IndexBufferPacked *indexBuffer, const Ogre::VertexElement2Vec& elemVec, int arraySize, const float* vertexData, Ogre::OperationType t);

        Ogre::MeshPtr mMeshPtr;
        Ogre::HlmsDatablock* mBlock;
        duDebugDrawPrimitives mOpType;
        float mVertOffset;
        std::vector<float> mVertices;
    };


    class NavMeshDebugDraw{
    public:
        NavMeshDebugDraw();
        ~NavMeshDebugDraw();

        void _createFromMesh();

        Ogre::MeshPtr produceMeshForNavMesh(dtNavMesh* mesh);
    };
}

#endif
