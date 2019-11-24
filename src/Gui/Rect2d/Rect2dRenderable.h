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
    };
}
