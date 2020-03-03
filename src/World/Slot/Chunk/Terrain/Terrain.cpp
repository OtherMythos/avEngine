#include "Terrain.h"

#include "terra/Terra.h"
#include "terra/Hlms/OgreHlmsTerraDatablock.h"
#include "OgreHlms.h"

#include "System/SystemSetup/SystemSettings.h"
#include "World/Physics/PhysicsShapeManager.h"
#include "World/Physics/PhysicsBodyConstructor.h"

#include "filesystem/path.h"

#include <Ogre.h>
#include "Logger/Log.h"

namespace AV{

    Ogre::TexturePtr Terrain::mShadowMap;

    Terrain::Terrain(){

    }

    Terrain::~Terrain(){
        if(mSetupComplete){
            teardown();
        }
        if(mTerra){
            delete mTerra;
        }
    }

    void Terrain::update(){
        if(mSetupComplete){
            mTerra->update();
        }
    }

    void Terrain::_createTerrainResourceGroup(const Ogre::String& dirPath, const Ogre::String& groupName){
        Ogre::ResourceGroupManager::getSingleton().createResourceGroup(groupName, false);
        Ogre::Root::getSingleton().addResourceLocation(dirPath, "FileSystem", groupName); //The resource group is the coordinates of this chunk.

        Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup(groupName, false);
    }

    void Terrain::_applyBlendMapToDatablock(Ogre::HlmsTerraDatablock* db){
        Ogre::TexturePtr blendMapTex = db->getTexture(Ogre::TERRA_DETAIL_WEIGHT);

        if(blendMapTex) return; //If the datablock already has a texture there's nothing to do.

        //If no blend map was provided, we try and take the one from the directory.
        //This is the generally expected outcome.
        if(Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroupName, "detailMap.png")){
            Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().load("detailMap.png", mTerrainGroupName, Ogre::TEX_TYPE_2D_ARRAY);

            //OPTIMISATION In future I could alter the json loader to find this texture immediately.
            //That would avoid a potential re-gen of the shaders.
            db->setTexture(Ogre::TERRA_DETAIL_WEIGHT, 0, tex);
        }else{
            AV_WARN("No detail map was provided or found for the terrain {}", mTerrainGroupName);
        }
    }

    Ogre::HlmsDatablock* Terrain::_getTerrainDatablock(const ChunkCoordinate& coord){
        Ogre::Root* root = Ogre::Root::getSingletonPtr();

        const std::string groupName = coord.getTerrainGroupName();

        //Here I don't actually do any checks as to whether that datablock exists exactly within the group.
        //That would be slightly slower, but it can be done in the future if needs be.
        Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms("Terra");
        Ogre::HlmsDatablock* datablock = hlms->getDatablock(groupName);

        if(!datablock){
            //If the user didn't define a datablock for this chunk then just use the default one.
            datablock = hlms->getDefaultDatablock();
        }else{
            //Checks for the blend map.
            Ogre::HlmsTerraDatablock* tBlock = dynamic_cast<Ogre::HlmsTerraDatablock*>(datablock);
            _applyBlendMapToDatablock(tBlock);
        }

        return datablock;
    }

    void Terrain::provideSceneNode(Ogre::SceneNode* node){
        //TODO
        //In future I plan to have the scene node be passed in at setup.
        //Terrain instances and instances of terra are going to be recycled between chunks, so setup is going to try and reduce the ammount of stuff it does.

        mNode = node;
    }

    void Terrain::_resetVals(){
        mTerrainGroupName.clear();
        mGroupPath.clear();
        mCurrentSetDatablock = 0;
    }

    void Terrain::teardown(){
        assert(mSetupComplete && "Teardown called when the terrain was not setup.");

        Ogre::Root& root = Ogre::Root::getSingleton();
        Ogre::Hlms* terraHlms = root.getHlmsManager()->getHlms("Terra");

        Ogre::HlmsDatablock* defaultDb = terraHlms->getDefaultDatablock();

        if(mTerra){
            mTerra->setDatablock(defaultDb);
            mNode->detachObject(mTerra);
        }

        if(defaultDb != mCurrentSetDatablock){ //We don't want to destroy the default datablock.
            terraHlms->destroyDatablock(mTerrainGroupName);
        }
        //Note: I'm not actually destroying any of the other datablocks that were loaded as part of this group.
        //This is purely for the sake of efficiency, as in order to do that you would have to traverse every single block in the manager to check its group.
        //So the user needs to be careful when putting things in the terrain directory, as if the chunk is loaded again they're very likely to get assertions due to duplicates.

        root.removeResourceLocation(mGroupPath);
        Ogre::ResourceGroupManager::getSingleton().destroyResourceGroup(mTerrainGroupName);

        mSetupComplete = false;
    }

    Ogre::TexturePtr Terrain::_getBlankShadowMap(){
        if(!mShadowMap){
            using namespace Ogre;
            //The shadow map was never created, so we need to do that now.
            mShadowMap = TextureManager::getSingleton().createManual(
                        "terrainEmptyShadowMap",
                        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                        TEX_TYPE_2D, 1, 1,
                        0, PF_A2B10G10R10, TU_STATIC_WRITE_ONLY );

            v1::HardwarePixelBufferSharedPtr pixelBufferBuf = mShadowMap->getBuffer(0);
            const PixelBox &currImage = pixelBufferBuf->lock( Box( 0, 0,
                                                                   pixelBufferBuf->getWidth(),
                                                                   pixelBufferBuf->getHeight() ),
                                                              v1::HardwareBuffer::HBL_DISCARD );

            Ogre::uint32* pDest = static_cast<Ogre::uint32*>(currImage.data);

            static const Ogre::ColourValue blankColour(1, 0, 1, 1); //Magenta because the shadow map uses it for transparent
            for(int y = 0; y < pixelBufferBuf->getHeight(); y++){
                for(int x = 0; x < pixelBufferBuf->getWidth(); x++){
                    PixelUtil::packColour(blankColour, mShadowMap->getFormat(), pDest);
                    pDest++;
                }
            }


            pixelBufferBuf->unlock();
        }

        return mShadowMap;
    }

    bool Terrain::setup(const ChunkCoordinate& coord){
        assert(mNode && "Make sure to provide the terrain with a scene node before calling setup.");
        assert(!mSetupComplete && "Setup called when the terrain is already setup");
        _resetVals();

        #ifdef __APPLE__
            AV_WARN("Terrain is currently disabled on MacOS. "
                        "This is due to a lack of terrain support in ogre 2.1. "
                        "When I switch the engine to 2.2 I will include support for it then. ");
            return false;
        #endif

        const Ogre::String chunkPathString(SystemSettings::getMapsDirectory() + "/" + coord.getFilePath() + "/terrain");
        //Within the chunk path there should be a directory named terrain. If this doesn't exist then this chunk has no terrain.
        const filesystem::path chunkPath(chunkPathString);
        if(!chunkPath.exists()) return false;
        if(!chunkPath.is_directory()) return false;

        const filesystem::path heightPath = chunkPath / filesystem::path("height.png");
        if(!heightPath.exists() || !heightPath.is_file()) return false; //The only file we absolutely need to enable the terrain is the heightmap.


        //Everything is in place, so start with the terrain.
        mTerrainGroupName = coord.getTerrainGroupName();
        mGroupPath = chunkPathString;

        //If that directory exists, create a new resource group within it. This will allow us to safely collect all the resources specific to that directory.
        _createTerrainResourceGroup(chunkPathString, mTerrainGroupName);

        Ogre::Image img;
        img.load("height.png", mTerrainGroupName);

        Ogre::TexturePtr shadowTex;
        if(Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroupName, "shadow.png")){
            shadowTex = Ogre::TextureManager::getSingleton().load("shadow.png", mTerrainGroupName);
        }else{
            shadowTex = _getBlankShadowMap();
        }

        if(!mTerra){
            //The actual terra instance is created lazily, so that if none of the terrains ever pass the tests above the memory isn't wasted.
            Ogre::Root& root = Ogre::Root::getSingleton();

            //TODO remove all these getter functions.
            Ogre::SceneManager* mgr = root.getSceneManager("Scene Manager");

            mTerra = new Ogre::Terra( Ogre::Id::generateNewId<Ogre::MovableObject>(),
                                            &mgr->_getEntityMemoryManager( Ogre::SCENE_STATIC ),
                                            mgr, 0, Ogre::Root::getSingletonPtr()->getCompositorManager2(),
                                            mgr->getCameras()[0]);

            mTerra->setCastShadows( false );
        }


        const Ogre::Vector3 nPos = mNode->getPosition();
        int slotSize = SystemSettings::getWorldSlotSize();

        //Determine the correct scale and position values to add to the terrain sides.
        //I was having an issue where the terrain wasn't really honoring the size of the chunks, and there would be a slight gap between each terrain.
        //From asking on the forums it seemed that one vertice was missing, so this finds that here.
        //It determines the size of a single vertice on the terrain and scales and centres based on this.
        const Ogre::Real rel = (float)slotSize / (float)img.getWidth();
        const Ogre::Real pos = slotSize / 2 + rel / 2;

        mTerra->load( img, shadowTex, Ogre::Vector3(nPos.x + pos, 0, nPos.z + pos), Ogre::Vector3(slotSize + rel, slotSize, slotSize + rel));

        Ogre::HlmsDatablock *datablock = _getTerrainDatablock(coord);
        //Seems you have to set the datablock after the load.
        //Otherwise when you try and set it, it misses the renderables because they don't exist.
        mTerra->setDatablock( datablock );
        mCurrentSetDatablock = datablock;

        mNode->attachObject( mTerra );
        mSetupComplete = true;


        //Create the physics object.
        Ogre::uint32 terrainSize = mTerra->getTerrainWidth();


        //In future this stuff should be determined automatically.
        static const Ogre::uint32 terrainDiv = 1;
        const Ogre::uint32 terrainRes = terrainSize / terrainDiv;
        const std::vector<float>& terrainData = mTerra->getHeightData();
        //float* data = new float[terrainRes * terrainRes];
        const float* data = &(terrainData[0]);

        //assert(terrainRes * terrainRes == terrainData.size());
        //float* counterData = data;
        //int currentReference = 0;
        /*for(int i = 0; i < terrainRes * terrainRes; i++){
            *counterData++ = terrainData[currentReference];
            currentReference += terrainDiv;
        }*/

        //mTerrainShape = PhysicsShapeManager::getTerrainShape(terrainRes, terrainRes, (const void*)data, 100.0f, 0, 100);
        mTerrainShape = PhysicsShapeManager::getTerrainShape(terrainRes, terrainRes, (const void*)data, 0, mTerra->getHeight(), float(slotSize) / float(terrainRes));
        //mTerrainShape = PhysicsShapeManager::getTerrainShape(1024, 1024, (const void*)data, 100.0f/1024.0f, 0, 100);

        //btVector3 localScaling(float(slotSize) / float(terrainRes), 1.0, float(slotSize) / float(terrainRes));
        float halfTerrainSize = float(slotSize) / 2;
        btVector3 terrainOrigin(nPos.x + halfTerrainSize, 0, nPos.y + halfTerrainSize);
        mTerrainBody = PhysicsBodyConstructor::createTerrainBody(mTerrainShape, terrainOrigin);
        //btHeightfieldTerrainShape* terrainShape = new btHeightfieldTerrainShape(500, 500, (void*)data, 1, -100, 100, 1, PHY_FLOAT, false);

        return true;
    }

    void Terrain::clearShadowTexture(){
        if(mShadowMap) mShadowMap.setNull();
    }
}
