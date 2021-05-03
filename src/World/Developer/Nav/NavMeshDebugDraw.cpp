#ifdef DEBUGGING_TOOLS

#include "NavMeshDebugDraw.h"

#include "Ogre.h"
#include <OgreMeshManager2.h>
#include <OgreMesh2.h>
#include "OgreSubMesh2.h"
#include "Vao/OgreVaoManager.h"

namespace AV{

    SouthseaDebugDraw::SouthseaDebugDraw() {
        mMeshPtr.reset();
    }

    SouthseaDebugDraw::~SouthseaDebugDraw(){

    }

    Ogre::MeshPtr NavMeshDebugDraw::produceMeshForNavMesh(dtNavMesh* mesh){
        SouthseaDebugDraw draw;
        duDebugDrawNavMesh(&draw, *mesh, DU_DRAWNAVMESH_OFFMESHCONS);
        Ogre::MeshPtr createdMesh = draw.endMesh();

        return createdMesh;
    }


    void SouthseaDebugDraw::begin(duDebugDrawPrimitives prim, float size){
        //std::cout << "begin drawing" << std::endl;
        mVertices.clear();
        mOpType = prim;
    }

    void SouthseaDebugDraw::_vertex(float x, float y, float z, unsigned int color){
        mVertices.push_back(x);
        mVertices.push_back(y);
        mVertices.push_back(z);

        //std::cout << std::bitset<32>(color) << std::endl;
        float r = float((color>>24)&0xFF) / 255.0f;
        float g = float((color>>16)&0xFF) / 255.0f;
        float b = float((color>>8)&0xFF) / 255.0f;
        // std::cout << r << std::endl;
        // std::cout << g << std::endl;
        // std::cout << b << std::endl;
        mVertices.push_back(r);
        mVertices.push_back(g);
        mVertices.push_back(b);
        mVertices.push_back(0.7f);
    }

    void SouthseaDebugDraw::vertex(const float* pos, unsigned int color){
        _vertex(*pos, *(pos+1), *(pos+2), color);
    }

    void SouthseaDebugDraw::vertex(const float x, const float y, const float z, unsigned int color){
        _vertex(x, y, z, color);
    }

    void SouthseaDebugDraw::vertex(const float* pos, unsigned int color, const float* uv){
        _vertex(*pos, *pos+1, *pos+2, color);
    }

    void SouthseaDebugDraw::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v){
        _vertex(x, y, z, color);
    }

    void SouthseaDebugDraw::end(){
        //Commit the vertices in the list to a mesh.
        //std::cout << "end" << std::endl;
        _createFromMesh();
    }

    void SouthseaDebugDraw::depthMask(bool state){

    }

    void SouthseaDebugDraw::texture(bool state){

    }



    NavMeshDebugDraw::NavMeshDebugDraw(){

    }

    NavMeshDebugDraw::~NavMeshDebugDraw(){

    }


    Ogre::IndexBufferPacked* createIndexBuffer(int cubeArraySize, const Ogre::uint32* indexData){
        Ogre::IndexBufferPacked *indexBuffer = 0;

        Ogre::uint32 *cubeIndices = reinterpret_cast<Ogre::uint32*>( OGRE_MALLOC_SIMD(sizeof(Ogre::uint32) * cubeArraySize, Ogre::MEMCATEGORY_GEOMETRY));
        //memcpy( cubeIndices, indexData, sizeof( *indexData ) );
        memcpy( cubeIndices, indexData, sizeof(Ogre::uint32) * cubeArraySize );

        Ogre::RenderSystem *renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();
        Ogre::VaoManager *vaoManager = renderSystem->getVaoManager();

        try{
            //Actually create an index buffer and assign it to the pointer created earlier.
            //Also populate the index buffer with these values.
            //This goes, type, number of indices, Buffer type, the actual data, keep as shadow
            indexBuffer = vaoManager->createIndexBuffer(Ogre::IndexBufferPacked::IT_32BIT, cubeArraySize, Ogre::BT_IMMUTABLE, cubeIndices, true);
        }
        catch(Ogre::Exception &e){
            OGRE_FREE_SIMD( indexBuffer, Ogre::MEMCATEGORY_GEOMETRY );
            indexBuffer = 0;
            throw e;
        }

        return indexBuffer;
    }


    void SouthseaDebugDraw::createStaticMesh(Ogre::IndexBufferPacked *indexBuffer, const Ogre::VertexElement2Vec& elemVec, int arraySize, const float* vertexData, Ogre::OperationType t){
        Ogre::RenderSystem *renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();
        Ogre::VaoManager *vaoManager = renderSystem->getVaoManager();

        if(!mMeshPtr){
            static int counter = 0;
            const std::string targetName = std::to_string(counter) + "navMesh";
            mMeshPtr = Ogre::MeshManager::getSingleton().createManual(targetName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            //TODO set this properly.
            mMeshPtr->_setBounds( Ogre::Aabb( Ogre::Vector3::ZERO, Ogre::Vector3(10, 10, 10) ), false );
            mMeshPtr->_setBoundingSphereRadius( 10.0f );
            counter++;
        }

        Ogre::SubMesh *subMesh = mMeshPtr->createSubMesh();

        float *cubeVertices = reinterpret_cast<float*>( OGRE_MALLOC_SIMD(sizeof(float) * arraySize, Ogre::MEMCATEGORY_GEOMETRY ) );

        memcpy(cubeVertices, vertexData, sizeof(float) * arraySize);

        Ogre::VertexBufferPacked *vertexBuffer = 0;
        try{
            vertexBuffer = vaoManager->createVertexBuffer(elemVec, arraySize, Ogre::BT_DEFAULT, cubeVertices, true);
        }catch(Ogre::Exception &e){
            vertexBuffer = 0;
            throw e;
        }

        Ogre::VertexBufferPackedVec vertexBuffers;
        vertexBuffers.push_back(vertexBuffer);

        Ogre::VertexArrayObject* vao = vaoManager->createVertexArrayObject(vertexBuffers, indexBuffer, t);

        subMesh->mVao[Ogre::VpNormal].push_back(vao);
        subMesh->mVao[Ogre::VpShadow].push_back(vao);
    }

    Ogre::MeshPtr SouthseaDebugDraw::endMesh(){
        assert(!mMeshPtr.isNull());

        Ogre::MeshPtr retMesh = mMeshPtr;
        mMeshPtr.reset();

        return retMesh;
    }

    void SouthseaDebugDraw::_createFromMesh(){
        if(mVertices.size() <= 0) return;

        int targetSize = mVertices.size() / 7;
        Ogre::uint32* indexData = new Ogre::uint32[targetSize];
        Ogre::uint32* count = indexData;
        for(Ogre::uint32 i = 0; i < targetSize; i++){
            //Some vertex positions might be submitted twice, so for the indices just draw them in a row.
            *count++ = i;
        }

        Ogre::IndexBufferPacked *indexBuffer = createIndexBuffer(targetSize, indexData);
        Ogre::VertexElement2Vec vertexElements;
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE));

        Ogre::OperationType opType;
        switch(mOpType){
            case duDebugDrawPrimitives::DU_DRAW_LINES:
                opType = Ogre::OT_LINE_LIST;
                break;
            case duDebugDrawPrimitives::DU_DRAW_POINTS:
                opType = Ogre::OT_POINT_LIST;
                break;
            default:
            case duDebugDrawPrimitives::DU_DRAW_QUADS:
                assert(false);
                break;
            case duDebugDrawPrimitives::DU_DRAW_TRIS:
                opType = Ogre::OT_TRIANGLE_LIST;
                break;
        }


        createStaticMesh(indexBuffer, vertexElements, mVertices.size(), &(mVertices[0]), opType);

    }
}

#endif
