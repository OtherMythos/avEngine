#include "InternalTextureManager.h"

#include "Ogre.h"
#include "OgreTextureGpuManager.h"
#include "OgreStagingTexture.h"

namespace AV{

    Ogre::TextureGpu* InternalTextureManager::mShadowMapTexture = 0;

    void InternalTextureManager::createTextures(){
        Ogre::TextureGpuManager* textureManager = Ogre::Root::getSingletonPtr()->getRenderSystem()->getTextureGpuManager();

        {
            const int width = 100;
            const int height = 100;
            mShadowMapTexture = textureManager->createTexture(
                "internal/terraShadowMap", Ogre::GpuPageOutStrategy::SaveToSystemRam,
                Ogre::TextureFlags::Uav, Ogre::TextureTypes::Type2D );
            mShadowMapTexture->setResolution(width, height);
            mShadowMapTexture->setPixelFormat( Ogre::PFG_RGBA8_UNORM );
            mShadowMapTexture->scheduleTransitionTo( Ogre::GpuResidency::Resident );

            Ogre::StagingTexture *stagingTexture = textureManager->getStagingTexture(width, height, mShadowMapTexture->getDepth(), mShadowMapTexture->getNumSlices(), mShadowMapTexture->getPixelFormat());
            stagingTexture->startMapRegion();
            Ogre::TextureBox texBox = stagingTexture->mapRegion(width, height, mShadowMapTexture->getDepth(), mShadowMapTexture->getNumSlices(), mShadowMapTexture->getPixelFormat());

            Ogre::uint8* pDest = static_cast<Ogre::uint8*>(texBox.at(0, 0, 0));
            Ogre::uint8* itPtr = pDest;
            for(int i = 0; i < width * height; i++){
                *itPtr++ = 0xFF;
                *itPtr++ = 0xFF;
                *itPtr++ = 0xFF;
                *itPtr++ = 0xFF;
            }
            //memcpy(pDest, out->waterTextureBufferMask, width * height * sizeof(float) * 4);

            stagingTexture->stopMapRegion();
            stagingTexture->upload(texBox, mShadowMapTexture, 0, 0, 0, false);

            textureManager->removeStagingTexture( stagingTexture );
            stagingTexture = 0;
        }

    }

    void InternalTextureManager::shutdown(){

    }

    Ogre::TextureGpu* InternalTextureManager::getTerraShadowMap(){
        return mShadowMapTexture;
    }

};
