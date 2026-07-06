#include "AVHlmsListenerDispatch.h"

#include "OgreHlms.h"
#include "OgreSceneManager.h"
#include "Compositor/Pass/OgreCompositorPass.h"
#include "Compositor/Pass/OgreCompositorPassDef.h"

#include <cstring>

namespace Ogre{

    AVHlmsListenerDispatch::AVHlmsListenerDispatch(){

    }

    AVHlmsListenerDispatch::~AVHlmsListenerDispatch(){

    }

    void AVHlmsListenerDispatch::registerListener(HlmsListener* listener){
        mExternalListeners.push_back(listener);
    }

    void AVHlmsListenerDispatch::removeListener(HlmsListener* listener){
        for(std::vector<HlmsListener*>::iterator it = mExternalListeners.begin(); it != mExternalListeners.end(); ++it){
            if(*it == listener){
                mExternalListeners.erase(it);
                return;
            }
        }
    }

    void AVHlmsListenerDispatch::setScriptBufferSize(size_t numFloats){
        mScriptFloats.resize(numFloats, 0.0f);
    }

    void AVHlmsListenerDispatch::setScriptFloat(size_t index, float value){
        if(index < mScriptFloats.size()){
            mScriptFloats[index] = value;
        }
    }

    void AVHlmsListenerDispatch::setScriptFloats(size_t offset, const float* values, size_t count){
        for(size_t i = 0; i < count && (offset + i) < mScriptFloats.size(); ++i){
            mScriptFloats[offset + i] = values[i];
        }
    }

    void AVHlmsListenerDispatch::setPassProperty(uint32 passIdentifier, IdString key, int32 value){
        std::vector<std::pair<IdString, int32>>& props = mPassProperties[passIdentifier];
        for(std::pair<IdString, int32>& p : props){
            if(p.first == key){
                p.second = value;
                return;
            }
        }
        props.push_back(std::make_pair(key, value));
    }

    void AVHlmsListenerDispatch::clearPassProperty(uint32 passIdentifier, IdString key){
        std::map<uint32, std::vector<std::pair<IdString, int32>>>::iterator mapIt = mPassProperties.find(passIdentifier);
        if(mapIt == mPassProperties.end()) return;
        std::vector<std::pair<IdString, int32>>& props = mapIt->second;
        for(std::vector<std::pair<IdString, int32>>::iterator it = props.begin(); it != props.end(); ++it){
            if(it->first == key){
                props.erase(it);
                return;
            }
        }
    }

    uint32 AVHlmsListenerDispatch::getPassBufferSize( const CompositorShadowNode *shadowNode, bool casterPass, bool dualParaboloid, SceneManager *sceneManager ) const {
        //The script float block is padded up to 16 bytes so that any external
        //listener chained after it still receives a 16-byte aligned pointer
        //(Ogre's per-listener contract). Padding it also satisfies the rule that
        //getPassBufferSize must be a multiple of 16 when the script buffer is the
        //only contributor. The trailing pad sits after custom_passBuffer (the last
        //region in the PassBuffer struct) so shaders simply ignore it.
        uint32 total = align16( static_cast<uint32>( mScriptFloats.size() * sizeof(float) ) );
        for(HlmsListener* l : mExternalListeners){
            total += l->getPassBufferSize(shadowNode, casterPass, dualParaboloid, sceneManager);
        }
        return total;
    }

    float* AVHlmsListenerDispatch::preparePassBuffer( const CompositorShadowNode *shadowNode, bool casterPass, bool dualParaboloid, SceneManager *sceneManager, float *passBufferPtr ){
        const size_t numScript = mScriptFloats.size();
        if(numScript > 0){
            memcpy(passBufferPtr, mScriptFloats.data(), numScript * sizeof(float));
            passBufferPtr += numScript;

            const uint32 tight  = static_cast<uint32>( numScript * sizeof(float) );
            const uint32 padded = align16( tight );
            const uint32 padFloats = ( padded - tight ) / static_cast<uint32>( sizeof(float) );
            for(uint32 i = 0; i < padFloats; ++i){
                *passBufferPtr++ = 0.0f;
            }
        }
        for(HlmsListener* l : mExternalListeners){
            passBufferPtr = l->preparePassBuffer(shadowNode, casterPass, dualParaboloid, sceneManager, passBufferPtr);
        }
        return passBufferPtr;
    }

    void AVHlmsListenerDispatch::preparePassHash( const CompositorShadowNode *shadowNode, bool casterPass, bool dualParaboloid, SceneManager *sceneManager, Hlms *hlms ){
        if(!mPassProperties.empty() && sceneManager){
            const CompositorPass* pass = sceneManager->getCurrentCompositorPass();
            if(pass){
                const uint32 identifier = pass->getDefinition()->mIdentifier;
                std::map<uint32, std::vector<std::pair<IdString, int32>>>::const_iterator it = mPassProperties.find(identifier);
                if(it != mPassProperties.end()){
                    for(const std::pair<IdString, int32>& p : it->second){
                        hlms->_setProperty(p.first, p.second);
                    }
                }
            }
        }
        for(HlmsListener* l : mExternalListeners){
            l->preparePassHash(shadowNode, casterPass, dualParaboloid, sceneManager, hlms);
        }
    }

    void AVHlmsListenerDispatch::hlmsTypeChanged( bool casterPass, CommandBuffer *commandBuffer, const HlmsDatablock *datablock, size_t texUnit ){
        for(HlmsListener* l : mExternalListeners){
            l->hlmsTypeChanged(casterPass, commandBuffer, datablock, texUnit);
        }
    }

    void AVHlmsListenerDispatch::propertiesMergedPreGenerationStep( Hlms *hlms, const HlmsCache &passCache, const HlmsPropertyVec &renderableCacheProperties, const PiecesMap renderableCachePieces[NumShaderTypes], const HlmsPropertyVec &properties, const QueuedRenderable &queuedRenderable ){
        for(HlmsListener* l : mExternalListeners){
            l->propertiesMergedPreGenerationStep(hlms, passCache, renderableCacheProperties, renderableCachePieces, properties, queuedRenderable);
        }
    }

    uint16 AVHlmsListenerDispatch::getNumExtraPassTextures( const HlmsPropertyVec &properties, bool casterPass ) const {
        uint16 total = 0;
        for(HlmsListener* l : mExternalListeners){
            total = static_cast<uint16>( total + l->getNumExtraPassTextures(properties, casterPass) );
        }
        return total;
    }

    void AVHlmsListenerDispatch::setupRootLayout( RootLayout &rootLayout, const HlmsPropertyVec &properties ) const {
        for(HlmsListener* l : mExternalListeners){
            l->setupRootLayout(rootLayout, properties);
        }
    }

    void AVHlmsListenerDispatch::shaderCacheEntryCreated( const String &shaderProfile, const HlmsCache *hlmsCacheEntry, const HlmsCache &passCache, const HlmsPropertyVec &properties, const QueuedRenderable &queuedRenderable ){
        for(HlmsListener* l : mExternalListeners){
            l->shaderCacheEntryCreated(shaderProfile, hlmsCacheEntry, passCache, properties, queuedRenderable);
        }
    }
}
