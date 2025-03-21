#include "TerrainObject.h"

#include "Ogre.h"
#include "OgreImage2.h"
#include "OgreTextureGpuManager.h"
#include "terra/Terra.h"
#include "terra/Hlms/OgreHlmsTerra.h"
#include "terra/Hlms/OgreHlmsTerraDatablock.h"

namespace AV{

    TerrainObject::TerrainObject(Ogre::SceneManager* sceneManager, Ogre::Camera* camera)
    : mSceneManager(sceneManager){

        mTerra = new Ogre::Terra( Ogre::Id::generateNewId<Ogre::MovableObject>(),
                                 &mSceneManager->_getEntityMemoryManager( Ogre::SCENE_STATIC ),
                                 mSceneManager, 0, Ogre::Root::getSingletonPtr()->getCompositorManager2(),
                                 camera, false);

    }

    void TerrainObject::load(const std::string& textureName, const Ogre::Vector3& origin, const Ogre::Vector3& size){

            Ogre::TextureGpuManager* manager = Ogre::Root::getSingletonPtr()->getRenderSystem()->getTextureGpuManager();

            Ogre::Image2 img;
            img.load(textureName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
            //manager->waitForStreamingCompletion();

            Ogre::uint32 tWidth = img.getWidth();
            Ogre::uint32 tDepth = img.getHeight();
            float* heightData = static_cast<float*>(malloc(tWidth * tDepth * sizeof(float)));

            mTerra->load(img, heightData, origin, size, false, false);

            //mTerra->setRenderQueueGroup(30);

            Ogre::Root& root = Ogre::Root::getSingleton();
            Ogre::Hlms* terraHlms = root.getHlmsManager()->getHlms("Terra");

            Ogre::HlmsDatablock* defaultDb = terraHlms->getDefaultDatablock();

            Ogre::HlmsTerraDatablock* targetBlock;
            if(false){
                Ogre::HlmsDatablock* db = terraHlms->createDatablock(Ogre::IdString("internalTerra"), "internalTerra", Ogre::HlmsMacroblock(), Ogre::HlmsBlendblock(), Ogre::HlmsParamVec(), false);
                    //Ogre::HlmsTerraDatablock* dbt = reinterpret_cast<Ogre::HlmsTerraDatablock*>(db);

                //Ogre::HlmsDatablock* dbb = terraHlms->createDatablock(Ogre::IdString("internalSecond"), "internalSecond", Ogre::HlmsMacroblock(), Ogre::HlmsBlendblock(), Ogre::HlmsParamVec(), false);

                //Ogre::HlmsDatablock* dbbbb = terraHlms->createDatablock(Ogre::IdString("internalThird"), "internalThird", Ogre::HlmsMacroblock(), Ogre::HlmsBlendblock(), Ogre::HlmsParamVec(), false);

                //Ogre::HlmsDatablock* dbbbbb = terraHlms->createDatablock(Ogre::IdString("iii"), "iii", Ogre::HlmsMacroblock(), Ogre::HlmsBlendblock(), Ogre::HlmsParamVec(), false);

                targetBlock = dynamic_cast<Ogre::HlmsTerraDatablock*>(db);
                targetBlock->setDiffuse(Ogre::Vector3(0, 0, 0));
            }

            //Ogre::HlmsTerraDatablock* defaultTerra = reinterpret_cast<Ogre::HlmsTerraDatablock*>(defaultDb);
            //targetBlock->setDiffuse(Ogre::Vector3(1, 1, 1));
            //Ogre::HlmsMacroblock macro;
            //macro.mPolygonMode = Ogre::PM_WIREFRAME;
            //targetBlock->setMacroblock(macro);

            /*
            Ogre::TextureGpu* tex = 0;
            tex = manager->createOrRetrieveTexture("waterBlue.png", Ogre::GpuPageOutStrategy::Discard, Ogre::TextureFlags::AutomaticBatching, Ogre::TextureTypes::Type2D, "General");
            tex->scheduleTransitionTo(Ogre::GpuResidency::Resident);
            manager->waitForStreamingCompletion();

            targetBlock->setTexture(Ogre::TERRA_DIFFUSE, tex);
             */
            //dbbbbb->setMetalness(0, 0);
            //mTerra->setDatablock(targetBlock);
            mTerra->setDatablock(defaultDb);
            //mTerra->setDatablock(targetBlock);

            //mTerra->setDatablock(db);

            mTerra->setCastShadows( false );

            //mSceneManager->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_STATIC)->attachObject(mTerra);
    }

    TerrainObject::~TerrainObject(){

    }

    void TerrainObject::update(){
        mTerra->update();
    }

}
