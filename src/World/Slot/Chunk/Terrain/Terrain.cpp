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

        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        Ogre::SceneManager* mgr = root->getSceneManager("Scene Manager");

        //In future this might be moved to setup.
        mTerra = new Ogre::Terra( Ogre::Id::generateNewId<Ogre::MovableObject>(),
                                        &mgr->_getEntityMemoryManager( Ogre::SCENE_STATIC ),
                                        mgr, 0, Ogre::Root::getSingletonPtr()->getCompositorManager2(),
                                        mgr->getCameras()[0]);

        mTerra->setCastShadows( false );
    }

    Terrain::~Terrain(){

    }

    void Terrain::update(){
        //From the looks of it, the second value isn't used for anything other than determining whether the shadow map should be updated.
        //It's updated once every frame if the value is set to 0.
        //Setting it to 1 means the shadow map should only update once on creation, and I guess you could dynamically adjust this value depending on when you want to update it.
        if(mSetupComplete){
            mTerra->update( Ogre::Vector3( -1, -1, -1 ).normalisedCopy(), 1);
        }
    }

    void Terrain::_createTerrainResourceGroup(const Ogre::String& dirPath, const Ogre::String& groupName){
        Ogre::ResourceGroupManager::getSingleton().createResourceGroup(groupName, false);
        Ogre::Root::getSingleton().addResourceLocation(dirPath, "FileSystem", groupName); //The resource group is the coordinates of this chunk.

        Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup(groupName, false);
    }

    Ogre::HlmsDatablock* Terrain::_getTerrainDatablock(const ChunkCoordinate& coord){
        Ogre::Root* root = Ogre::Root::getSingletonPtr();

        const std::string groupName = coord.getTerrainGroupName();

        //Here I don't actually do any checks as to whether that datablock exists exactly within the group.
        //That would be slightly slower, but it can be done in the future if needs be.
        Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms("Terra");
        Ogre::HlmsDatablock* datablock = hlms->getDatablock(groupName);
        Ogre::HlmsTerraDatablock* tBlock = dynamic_cast<Ogre::HlmsTerraDatablock*>(datablock);

        { //Checks for the detail map.

            Ogre::TexturePtr blendMapTex = tBlock->getTexture(Ogre::TERRA_DETAIL_WEIGHT);

            if(!blendMapTex){
                //If no blend map was provided, we try and take the one from the directory.
                //This is the generally expected outcome.
                if(Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroupName, "detailMap.png")){
                    Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().load("detailMap.png", mTerrainGroupName, Ogre::TEX_TYPE_2D_ARRAY);

                    //OPTIMISATION In future I could alter the json loader to find this texture immediately.
                    //That would avoid a potential re-gen of the shaders.
                    tBlock->setTexture(Ogre::TERRA_DETAIL_WEIGHT, 0, tex);
                }else{
                    AV_WARN("No detail map was provided or found for the terrain {}", groupName);
                }
            }
        }

        return datablock;
    }

    void Terrain::provideSceneNode(Ogre::SceneNode* node){
        //In future I plan to have the scene node be passed in at setup.
        //Terrain instances and instances of terra are going to be recycled between chunks, so setup is going to try and reduce the ammount of stuff it does.

        mNode = node;
    }

    void Terrain::_resetVals(){
        mSetupComplete = false;
        mTerrainGroupName.clear();
    }

    bool Terrain::setup(const ChunkCoordinate& coord){
        assert(mNode && "Make sure to provide the terrain with a scene node before calling setup.");
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

        //If that directory exists, create a new resource group within it. This will allow us to safely collect all the resources specific to that directory.
        _createTerrainResourceGroup(chunkPathString, mTerrainGroupName);

        Ogre::Image img;
        img.load("height.png", mTerrainGroupName);

        Ogre::Image shadowImg;
        Ogre::Image* passShadowImage = 0;
        if(Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroupName, "shadow.png")){
            shadowImg.load("shadow.png", mTerrainGroupName);
            passShadowImage = &shadowImg;
        }


        const Ogre::Vector3 nPos = mNode->getPosition();
        int slotSize = SystemSettings::getWorldSlotSize();
        mTerra->load( img, passShadowImage, Ogre::Vector3(nPos.x, 0, nPos.z), Ogre::Vector3(slotSize, slotSize, slotSize));

        Ogre::HlmsDatablock *datablock = _getTerrainDatablock(coord);
        //Seems you have to set the datablock after the load.
        //Otherwise when you try and set it, it misses the renderables because they don't exist.
        mTerra->setDatablock( datablock );

        mNode->attachObject( mTerra );

        mSetupComplete = true;

        return true;
    }
}
