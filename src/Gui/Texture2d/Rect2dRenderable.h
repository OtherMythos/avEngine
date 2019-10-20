#pragma once

#include <OgreRenderable.h>
#include <OgreRenderOperation.h>

namespace AV{
    class Rect2dRenderable : public Ogre::Renderable {
    public:
        Rect2dRenderable();
        ~Rect2dRenderable();

        //Overrides from Renderable
        void getWorldTransforms(Ogre::Matrix4* xform) const;
        void getRenderOperation(Ogre::v1::RenderOperation& op, bool casterPass);
        bool getCastsShadows(void) const;

        const Ogre::LightList& getLights(void) const{
           static Ogre::LightList ll;
           return ll;
        };

    private:
        Ogre::VaoManager* vaoManager; /**< VAO manager used */
        Ogre::VertexArrayObject* vao; /**< The single VAO of this renderable */
        float* vertices; /**< The SIMD array of vertices */
        Ogre::VertexBufferPacked* vertexBuffer; /**< Its vertex buffer */
        Ogre::IndexBufferPacked* indexBuffer; /**< Its index buffer */

        void createVAO();
    };
}
