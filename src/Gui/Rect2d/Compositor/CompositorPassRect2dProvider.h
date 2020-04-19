#pragma once

#include "OgrePrerequisites.h"
#include "Compositor/Pass/OgreCompositorPassProvider.h"

namespace Colibri{
    class ColibriManager;
}

namespace AV{
    class CompositorPassRect2dProvider : public Ogre::CompositorPassProvider{
    public:
        CompositorPassRect2dProvider(Colibri::ColibriManager* colibriManager);

        virtual Ogre::CompositorPassDef* addPassDef( Ogre::CompositorPassType passType,
                                       Ogre::IdString customId,
                                       Ogre::CompositorTargetDef *parentTargetDef,
                                       Ogre::CompositorNodeDef *parentNodeDef );

        virtual Ogre::CompositorPass* addPass( const Ogre::CompositorPassDef *definition, Ogre::Camera *defaultCamera,
                                         Ogre::CompositorNode *parentNode, const Ogre::RenderTargetViewDef* target,
                                         Ogre::SceneManager *sceneManager );

    private:
        Colibri::ColibriManager* mColibriManager;
    };
}
