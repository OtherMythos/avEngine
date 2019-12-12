#include "CompositorPassRect2dProvider.h"

#include "CompositorPassRect2dDef.h"
#include "CompositorPassRect2d.h"

namespace AV{
    CompositorPassRect2dProvider::CompositorPassRect2dProvider(){

    }

    Ogre::CompositorPassDef* CompositorPassRect2dProvider::addPassDef( Ogre::CompositorPassType passType,
                                   Ogre::IdString customId,
                                   Ogre::CompositorTargetDef *parentTargetDef,
                                   Ogre::CompositorNodeDef *parentNodeDef){
        if(customId == "rect2d"){
            return OGRE_NEW CompositorPassRect2dDef(parentTargetDef);
        }

        return 0;
    }

    Ogre::CompositorPass* CompositorPassRect2dProvider::addPass( const Ogre::CompositorPassDef *definition, Ogre::Camera *defaultCamera,
                                     Ogre::CompositorNode *parentNode, const Ogre::CompositorChannel& target,
                                     Ogre::SceneManager *sceneManager){
        const CompositorPassRect2dDef *rect2dDef = static_cast<const CompositorPassRect2dDef*>( definition );
        return OGRE_NEW CompositorPassRect2d(rect2dDef, sceneManager, target, parentNode);
    }
}
