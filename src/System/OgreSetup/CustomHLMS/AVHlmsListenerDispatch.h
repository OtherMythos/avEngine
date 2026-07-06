#pragma once

#include "OgreHlmsListener.h"
#include "OgreIdString.h"

#include <vector>
#include <map>
#include <utility>

namespace Ogre{

    /**
    A single Ogre::HlmsListener that fans out to a list of registered listeners,
    working around Ogre's one-listener-per-Hlms limit (Hlms::setListener replaces
    rather than appends). One of these is set on each Hlms during engine setup and
    can be retrieved again with Hlms::getListener().

    It also owns the two script-facing features directly, so Squirrel can drive
    shaders with no C++:
      * a pass buffer of floats (getPassBufferSize / preparePassBuffer) exposed
        through the custom_passBuffer shader piece, and
      * per-compositor-pass shader properties applied in preparePassHash (keyed by
        the pass's mIdentifier), for selecting shader variants per pass.

    Plugins / engine subsystems can still contribute a full HlmsListener via
    registerListener(); those are invoked after the script data.
    */
    class AVHlmsListenerDispatch : public HlmsListener{
    public:
        AVHlmsListenerDispatch();
        ~AVHlmsListenerDispatch();

        //--- external listener management (plugins, engine subsystems) ---
        void registerListener(HlmsListener* listener);
        void removeListener(HlmsListener* listener);

        //--- script pass buffer (the custom_passBuffer float block) ---
        /// Resize the script float block; existing values below the new size are
        /// preserved, new entries are zeroed.
        void setScriptBufferSize(size_t numFloats);
        size_t getScriptBufferSize() const { return mScriptFloats.size(); }
        void setScriptFloat(size_t index, float value);
        void setScriptFloats(size_t offset, const float* values, size_t count);

        //--- per-pass shader properties (preparePassHash) ---
        void setPassProperty(uint32 passIdentifier, IdString key, int32 value);
        void clearPassProperty(uint32 passIdentifier, IdString key);

        //--- Ogre::HlmsListener overrides ---
        uint32 getPassBufferSize( const CompositorShadowNode *shadowNode, bool casterPass,
                                  bool dualParaboloid, SceneManager *sceneManager ) const override;
        float* preparePassBuffer( const CompositorShadowNode *shadowNode, bool casterPass,
                                  bool dualParaboloid, SceneManager *sceneManager,
                                  float *passBufferPtr ) override;
        void preparePassHash( const CompositorShadowNode *shadowNode, bool casterPass,
                              bool dualParaboloid, SceneManager *sceneManager, Hlms *hlms ) override;
        void hlmsTypeChanged( bool casterPass, CommandBuffer *commandBuffer,
                              const HlmsDatablock *datablock, size_t texUnit ) override;
        void propertiesMergedPreGenerationStep(
            Hlms *hlms, const HlmsCache &passCache, const HlmsPropertyVec &renderableCacheProperties,
            const PiecesMap renderableCachePieces[NumShaderTypes], const HlmsPropertyVec &properties,
            const QueuedRenderable &queuedRenderable ) override;
        uint16 getNumExtraPassTextures( const HlmsPropertyVec &properties,
                                        bool casterPass ) const override;
        void setupRootLayout( RootLayout &rootLayout, const HlmsPropertyVec &properties ) const override;
        void shaderCacheEntryCreated( const String &shaderProfile, const HlmsCache *hlmsCacheEntry,
                                      const HlmsCache &passCache, const HlmsPropertyVec &properties,
                                      const QueuedRenderable &queuedRenderable ) override;

    private:
        /// Round a byte count up to the next multiple of 16 (Ogre pass-buffer rule).
        static uint32 align16( uint32 v ){ return ( v + 15u ) & ~15u; }

        std::vector<HlmsListener*> mExternalListeners;
        std::vector<float> mScriptFloats;
        std::map<uint32, std::vector<std::pair<IdString, int32>>> mPassProperties;
    };
}
