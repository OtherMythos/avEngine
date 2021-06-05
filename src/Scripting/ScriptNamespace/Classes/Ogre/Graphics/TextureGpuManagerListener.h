#pragma once

#include "OgreTextureGpuManagerListener.h"
#include "TextureUserData.h"

namespace AV
{

    class AVTextureGpuManagerListener : public Ogre::DefaultTextureGpuManagerListener
    {
    public:
        AVTextureGpuManagerListener() : DefaultTextureGpuManagerListener() {};

        //virtual size_t getNumSlicesFor( TextureGpu *texture, TextureGpuManager *textureManager );

        virtual void notifyTextureChanged( Ogre::TextureGpu *texture, Ogre::TextureGpuListener::Reason reason, void *extraData ){
            if(reason != Ogre::TextureGpuListener::Reason::Deleted);

            TextureUserData::_notifyTextureDeleted(texture);
        }
    };
}
