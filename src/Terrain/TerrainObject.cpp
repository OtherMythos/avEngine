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
        //TODO clean up this block of memory :3
        float* heightData = static_cast<float*>(malloc(tWidth * tDepth * sizeof(float)));

        mTerra->load(img, heightData, origin, size, false, false);

        Ogre::Root& root = Ogre::Root::getSingleton();
        Ogre::Hlms* terraHlms = root.getHlmsManager()->getHlms("Terra");

        Ogre::HlmsDatablock* defaultDb = terraHlms->getDefaultDatablock();

#if 0
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
        Ogre::HlmsMacroblock macro;
        macro.mPolygonMode = Ogre::PM_WIREFRAME;
        //defaultDb->setMacroblock(macro);

        Ogre::TextureGpu* tex = 0;
        tex = manager->createOrRetrieveTexture("waterWaves.png", Ogre::GpuPageOutStrategy::Discard, Ogre::TextureFlags::AutomaticBatching, Ogre::TextureTypes::Type2D, "General");
        tex->scheduleTransitionTo(Ogre::GpuResidency::Resident);

        Ogre::TextureGpu* blueTex = 0;
        blueTex = manager->createOrRetrieveTexture("blueTexture", Ogre::GpuPageOutStrategy::Discard, Ogre::TextureFlags::AutomaticBatching, Ogre::TextureTypes::Type2D, "General");
        blueTex->scheduleTransitionTo(Ogre::GpuResidency::Resident);

        Ogre::TextureGpu* testTexture = 0;
        testTexture = manager->createOrRetrieveTexture("testTexture", Ogre::GpuPageOutStrategy::Discard, Ogre::TextureFlags::AutomaticBatching, Ogre::TextureTypes::Type2DArray);
        testTexture->scheduleTransitionTo(Ogre::GpuResidency::Resident);

        Ogre::TextureGpu* testTextureMask = 0;
        testTextureMask = manager->createOrRetrieveTexture("testTextureMask", Ogre::GpuPageOutStrategy::Discard, Ogre::TextureFlags::AutomaticBatching, Ogre::TextureTypes::Type2DArray);
        testTextureMask->scheduleTransitionTo(Ogre::GpuResidency::Resident);

        manager->waitForStreamingCompletion();


        Ogre::HlmsSamplerblock samplerblock;
        samplerblock.mU = Ogre::TAM_WRAP;
        samplerblock.mV = Ogre::TAM_WRAP;
        samplerblock.mW = Ogre::TAM_WRAP;
        samplerblock.mMagFilter = Ogre::FO_POINT;

        (dynamic_cast<Ogre::HlmsTerraDatablock*>(defaultDb))->setTexture(Ogre::TerraTextureTypes::TERRA_DIFFUSE, testTexture);
        (dynamic_cast<Ogre::HlmsTerraDatablock*>(defaultDb))->setTexture(Ogre::TerraTextureTypes::TERRA_DETAIL0, tex, root.getHlmsManager()->getSamplerblock(samplerblock));
        (dynamic_cast<Ogre::HlmsTerraDatablock*>(defaultDb))->setTexture(Ogre::TerraTextureTypes::TERRA_DETAIL_WEIGHT, testTextureMask);

        //dbbbbb->setMetalness(0, 0);
        //mTerra->setDatablock(targetBlock);
        (dynamic_cast<Ogre::HlmsTerraDatablock*>(defaultDb))->setDetailMapOffsetScale(0, Ogre::Vector4(0, 0, 20, 20));
        //mTerra->setDatablock(targetBlock);

        (dynamic_cast<Ogre::HlmsTerraDatablock*>(defaultDb))->setRoughness(0, 1.0);
        (dynamic_cast<Ogre::HlmsTerraDatablock*>(defaultDb))->setMetalness(0, 0.0);

        //(dynamic_cast<Ogre::HlmsTerraDatablock*>(defaultDb))->setDetailTriplanarMetalnessEnabled(true);
        //(dynamic_cast<Ogre::HlmsTerraDatablock*>(defaultDb))->setDetailTriplanarRoughnessEnabled(true);
        //(dynamic_cast<Ogre::HlmsTerraDatablock*>(defaultDb))->setDetailTriplanarNormalEnabled(true);
        //(dynamic_cast<Ogre::HlmsTerraDatablock*>(defaultDb))->setDetailTriplanarDiffuseEnabled(true);

        //mTerra->setDatablock(db);
#endif
        mTerra->setDatablock(defaultDb);
        mTerra->setCastShadows( false );
    }

    TerrainObject::~TerrainObject(){

    }

    void TerrainObject::update(){
        mTerra->update();
    }

}
