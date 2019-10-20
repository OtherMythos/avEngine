#include "Rect2dRenderable.h"

#include <Vao/OgreVaoManager.h>

#include "Ogre.h"

namespace AV{
    Rect2dRenderable::Rect2dRenderable() : Ogre::Renderable(){

        mHasSkeletonAnimation = false;
        mUseIdentityProjection = true;
        mUseIdentityView = true;

        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        Ogre::RenderSystem* renderSystem = root->getRenderSystem();
        vaoManager = renderSystem->getVaoManager();

        createVAO();
    }

    Rect2dRenderable::~Rect2dRenderable(){
        if(vao){
            vaoManager->destroyVertexArrayObject(vao);
            vao = NULL;
        }
        if(vertexBuffer){
            if(vertexBuffer->getMappingState() != Ogre::MS_UNMAPPED){
                vertexBuffer->unmap(Ogre::UO_UNMAP_ALL);
            }
            vaoManager->destroyVertexBuffer(vertexBuffer);
            vertexBuffer = NULL;
        }
        if(vertices){
            OGRE_FREE_SIMD(vertices, Ogre::MEMCATEGORY_GEOMETRY);
            vertices = NULL;
        }
        if(indexBuffer){
            vaoManager->destroyIndexBuffer(indexBuffer);
            indexBuffer = NULL;
        }
    }

    void Rect2dRenderable::getRenderOperation(Ogre::v1::RenderOperation& op, bool casterPass)
    {
       OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED,
             "Rect2dRenderable doesn't support the old v1::RenderOperations.",
             "AV::Rect2dRenderable::getRenderOperation");
    }

    void Rect2dRenderable::getWorldTransforms(Ogre::Matrix4* xform) const
    {
       OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED,
             "Rect2dRenderable doesn't implement getWorldTransforms.",
             "AV::Rect2dRenderable::getWorldTransforms");
    }

    bool Rect2dRenderable::getCastsShadows(void) const
    {
       OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED,
             "Rect2dRenderable doesn't implement getCastsShadows.",
             "AV::Rect2dRenderable::getCastShadows");

    }

    void Rect2dRenderable::createVAO(){
        /* Define our face index data */
        const Ogre::uint16 indexData[6] = { 0, 1, 2, 2, 3, 0 };

        /* Copy it to a SIMD array (the memory will be managed by the vao */
        Ogre::uint16* faceIndices = reinterpret_cast<Ogre::uint16*>(
              OGRE_MALLOC_SIMD(sizeof(Ogre::uint16) * 6,
                               Ogre::MEMCATEGORY_GEOMETRY) );
        memcpy(faceIndices, indexData, sizeof(indexData));

        /* Let's create the index buffer */
        try
        {
           indexBuffer = vaoManager->createIndexBuffer(
                 Ogre::IndexBufferPacked::IT_16BIT, 6, Ogre::BT_IMMUTABLE,
                 faceIndices, true);
        }
        catch(Ogre::Exception& e)
        {
           /* With exceptions, we should need to free it */
           OGRE_FREE_SIMD(indexBuffer, Ogre::MEMCATEGORY_GEOMETRY);
           indexBuffer = NULL;
           throw e;
        }

        /* Define our vertices elements: Positions and UVs */
        Ogre::VertexElement2Vec vertexElements;
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3,
                 Ogre::VES_POSITION));
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT2,
                 Ogre::VES_TEXTURE_COORDINATES));

        /* Define our coordinates */
        float x1 = 0.0f, y1 = 0.0f, x2 = 1.0f, y2 = -1.0f;
        //calculateCoordinates(x1, y1, x2, y2);

        /* Define our vertices (with UVs)  */
        const float faceVertices[4 * 5] =
        {
           x2, y2, 0.0f, 1.0f, 1.0f,
           x2, y1, 0.0f, 1.0f, 0.0f,
           x1, y1, 0.0f, 0.0f, 0.0f,
           x1, y2, 0.0f, 0.0f, 1.0f
       };

        /* Let's copy it to a SIMD array */
        vertices = reinterpret_cast<float*>(OGRE_MALLOC_SIMD(
                 sizeof(float) * 4 * 5, Ogre::MEMCATEGORY_GEOMETRY));
        memcpy(vertices, faceVertices, sizeof(float) * 4 * 5);

        /* And create our packed vertex buffer */
        try
        {
            vertexBuffer = vaoManager->createVertexBuffer(vertexElements, 4,
                  Ogre::BT_DYNAMIC_PERSISTENT, vertices, false);
        }
        catch(Ogre::Exception &e)
        {
           OGRE_FREE_SIMD(vertexBuffer, Ogre::MEMCATEGORY_GEOMETRY);
           vertexBuffer = NULL;
           throw e;
        }

        /* Finally, the Vao. */
        Ogre::VertexBufferPackedVec vertexBuffers;
        vertexBuffers.push_back(vertexBuffer);

        vao = vaoManager->createVertexArrayObject(vertexBuffers, indexBuffer,
              Ogre::OT_TRIANGLE_LIST);

        mVaoPerLod[Ogre::VpNormal].push_back(vao);
        mVaoPerLod[Ogre::VpShadow].push_back(vao);
    }

}
