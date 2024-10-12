#include "CompositorPassRect2dProvider.h"

#include "CompositorPassRect2dDef.h"
#include "CompositorPassRect2d.h"

#include "ColibriGui/Ogre/CompositorPassColibriGuiDef.h"
#include "ColibriGui/Ogre/CompositorPassColibriGui.h"

namespace AV{
    CompositorPassRect2dProvider::CompositorPassRect2dProvider(Colibri::ColibriManager* colibriManager)
        : mColibriManager(colibriManager) {

    }

    Ogre::CompositorPassDef* CompositorPassRect2dProvider::addPassDef( Ogre::CompositorPassType passType,
                                   Ogre::IdString customId,
                                   Ogre::CompositorTargetDef *parentTargetDef,
                                   Ogre::CompositorNodeDef *parentNodeDef){
        if(customId == "rect2d"){
            CompositorPassRect2dDef* def = OGRE_NEW CompositorPassRect2dDef(parentTargetDef);
            def->mIdentifier = 100;
            return def;
        }
        else if(customId == "colibri_gui"){
            Ogre::CompositorPassColibriGuiDef* def = OGRE_NEW Ogre::CompositorPassColibriGuiDef(parentTargetDef);
            def->mIdentifier = 101;
            return def;
        }

        return 0;
    }

    Ogre::CompositorPass* CompositorPassRect2dProvider::addPass( const Ogre::CompositorPassDef *definition, Ogre::Camera *defaultCamera,
                                     Ogre::CompositorNode *parentNode, const Ogre::RenderTargetViewDef* target,
                                     Ogre::SceneManager *sceneManager){

        if(definition->mIdentifier == 100){
            const CompositorPassRect2dDef *rect2dDef = static_cast<const CompositorPassRect2dDef*>( definition );
            return OGRE_NEW CompositorPassRect2d(rect2dDef, sceneManager, target, parentNode);
        }else if(definition->mIdentifier == 101){
            const Ogre::CompositorPassColibriGuiDef *colibriGuiDef = static_cast<const Ogre::CompositorPassColibriGuiDef*>( definition );
            return OGRE_NEW Ogre::CompositorPassColibriGui(colibriGuiDef, defaultCamera, sceneManager, target, parentNode, mColibriManager);
        }else assert(false);

        return 0;
    }
}
