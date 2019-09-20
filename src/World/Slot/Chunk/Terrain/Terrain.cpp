#include "Terrain.h"

#include "terra/Terra.h"
#include "terra/Hlms/OgreHlmsTerraDatablock.h"
#include "OgreHlms.h"

#include "System/SystemSetup/SystemSettings.h"

#include "filesystem/path.h"

#include <Ogre.h>
#include "Logger/Log.h"

namespace AV{
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

    bool Terrain::setup(const ChunkCoordinate& coord){
        assert(mNode && "Make sure to provide the terrain with a scene node before calling setup.");
        assert(!mSetupComplete && "Setup called when the terrain is already setup");
        _resetVals();

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

        return true;
    }
}
