#include "Rect2dRenderable.h"

#include <Vao/OgreVaoManager.h>
#include "World/Support/ProgrammaticMeshGenerator.h"

#include "Ogre.h"

namespace AV{
    Rect2dRenderable::Rect2dRenderable() : Ogre::Renderable(){

        mHasSkeletonAnimation = false;
        mUseIdentityProjection = true;
        mUseIdentityView = true;

        mVaoPerLod[Ogre::VpNormal].push_back(ProgrammaticMeshGenerator::getRect2dVao());
        mVaoPerLod[Ogre::VpShadow].push_back(ProgrammaticMeshGenerator::getRect2dVao());
    }

    Rect2dRenderable::~Rect2dRenderable(){

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

}
