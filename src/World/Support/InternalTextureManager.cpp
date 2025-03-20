#include "InternalTextureManager.h"

#include "Ogre.h"
#include "OgreTextureGpuManager.h"

namespace AV{

    Ogre::TextureGpu* InternalTextureManager::mShadowMapTexture = 0;

    void InternalTextureManager::createTextures(){
        Ogre::TextureGpuManager* textureManager = Ogre::Root::getSingletonPtr()->getRenderSystem()->getTextureGpuManager();

        {
            mShadowMapTexture = textureManager->createTexture(
                "internal/terraShadowMap", Ogre::GpuPageOutStrategy::SaveToSystemRam,
                Ogre::TextureFlags::Uav, Ogre::TextureTypes::Type2D );
            mShadowMapTexture->setResolution( 100, 100 );
            mShadowMapTexture->setPixelFormat( Ogre::PFG_RGBA8_UNORM );
            mShadowMapTexture->scheduleTransitionTo( Ogre::GpuResidency::Resident );
        }

    }

    void InternalTextureManager::shutdown(){

    }

    Ogre::TextureGpu* InternalTextureManager::getTerraShadowMap(){
        return mShadowMapTexture;
    }

};
