#pragma once

#include "Ogre.h"
#include "OgreAbiUtils.h"
#include "OgreWindow.h"
#include "OgreTextureGpuManager.h"
#include "OgreHlmsUnlit.h"
#include "OgreHlmsPbs.h"
#include "System/OgreSetup/CustomHLMS/OgreHlmsPbsAVCustom.h"
#include "ColibriGui/Ogre/OgreHlmsColibri.h"
#include "World/Slot/Chunk/Terrain/terra/Hlms/OgreHlmsTerra.h"

#include "System/SystemSetup/SystemSettings.h"
#include "filesystem/path.h"
#include "SetupHelpers.h"

#include "Compositor/OgreCompositorWorkspace.h"
#include <Compositor/OgreCompositorManager2.h>
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorNodeDef.h"
#include "Compositor/Pass/PassClear/OgreCompositorPassClearDef.h"

#include "Gui/Rect2d/Compositor/CompositorPassRect2dProvider.h"

#include "Gui/Rect2d/Rect2dMovable.h"

#include "Logger/OgreAVLogListener.h"

namespace AV {
    class Window;

    /**
     An interface to setup Ogre.

     Abstracting the setup details means the intricate setup details can be more platform specific.
     This would include tasks like setting up the hlms shaders and the compositor.
     The more diverse the supported platforms, the more these steps will differ, so its worth the abstraction.
     */
    class OgreSetup{
    public:
        OgreSetup() {}
        virtual ~OgreSetup() {}
        virtual Ogre::Root* setupRoot() = 0;
        virtual void setupOgreWindow(Window *window) = 0;
        //virtual void setupScene(Ogre::Root *root, Ogre::SceneManager **sceneManager, Ogre::Camera **camera) = 0;

        Ogre::Root* _setupBasicOgreRoot(){
            const Ogre::AbiCookie abiCookie = Ogre::generateAbiCookie();
            return new Ogre::Root(&abiCookie, "", "", "", "avEngine-" + SystemSettings::getProjectName());
        }

        void _registerHlmsPieceLibraryToHlms(const std::string& hlmsName, Ogre::ArchiveVec& archivePbsLibraryFolders){
            Ogre::ArchiveManager &archiveManager = Ogre::ArchiveManager::getSingleton();

            const std::vector<std::string>* lib = SystemSettings::getHlmsUserLibrary(hlmsName);
            if(lib == 0) return;

            for(const std::string& entry : *lib){
                Ogre::Archive *archiveLibrary = archiveManager.load(entry, "FileSystem", true );
                archivePbsLibraryFolders.push_back( archiveLibrary );
            }
        }

        void setupLogger(){
            //Disable Ogre's writing the logs to the file system.
            Ogre::LogManager* logManager = OGRE_NEW Ogre::LogManager();
            logManager->createLog("Ogre.log", true, false, true);
            OgreAVLogListener* avListener = new OgreAVLogListener();
            logManager->getDefaultLog()->addListener(avListener);
        }

        void setupOgreResources(Ogre::Root *root){

            //TODO separate these have to be added locations from the others with a different function.
            { //Process the essential files. These are processed regardless of whether an ogre resources file was found, as the engine assumes they exist.
                const Ogre::String& masterPath = SystemSettings::getMasterPath();
                const Ogre::String resGroupName = EngineFlags::ENGINE_RES_PREREQUISITE + "/General";

                //Add the essential files to the resource locations. This includes things like compositor files, shaders and so on.
                root->addResourceLocation(masterPath + "/essential/terrain", "FileSystem", resGroupName);
                root->addResourceLocation(masterPath + "/essential/terrain/GLSL", "FileSystem", resGroupName);
                //TODO make it so HLSL isn't included in a linux build, and the same for the other platforms.
                root->addResourceLocation(masterPath + "/essential/terrain/HLSL", "FileSystem", resGroupName);
                root->addResourceLocation(masterPath + "/essential/terrain/Metal", "FileSystem", resGroupName);
                root->addResourceLocation(masterPath + "/essential/common", "FileSystem", resGroupName);

                root->addResourceLocation(masterPath + "/essential/common/Any", "FileSystem", resGroupName);
                root->addResourceLocation(masterPath + "/essential/common/GLSL", "FileSystem", resGroupName);
                root->addResourceLocation(masterPath + "/essential/common/HLSL", "FileSystem", resGroupName);
                root->addResourceLocation(masterPath + "/essential/common/Metal", "FileSystem", resGroupName);

                root->addResourceLocation(masterPath + "/essential/compositor", "FileSystem", resGroupName);

                //Initialise these added groups earlier so any scripts added by the user later can reference them.
                Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup(resGroupName, false);
            }

            if(SystemSettings::isOgreResourcesFileViable()){
                SetupHelpers::parseOgreResourcesFile(SystemSettings::getOgreResourceFilePath());
            }

            //Add the locations listed in the setup file (if there are any).
            const std::vector<std::string>& resourceGroups = SystemSettings::getResourceGroupNames();
            const std::vector<SystemSettings::OgreResourceEntry>& groupEntries = SystemSettings::getResourceEntries();
            if(groupEntries.size() > 0){
                filesystem::path setupRelativePath(SystemSettings::getAvSetupFilePath());
                //setupRelativePath = relativePath.parent_path();

                for(const SystemSettings::OgreResourceEntry& e : groupEntries){
                    assert(e.groupId < resourceGroups.size());
                    SetupHelpers::addResourceLocation(e.path, resourceGroups[e.groupId], setupRelativePath);
                }
            }
        }

        void setupScene(Ogre::Root *root, Ogre::SceneManager **_sceneManager, Ogre::Camera **_camera){
            Ogre::SceneManager *sceneManager = root->createSceneManager(Ogre::ST_GENERIC, 2, "Scene Manager");

            Ogre::Camera *camera = sceneManager->createCamera("Main Camera");

            camera->setNearClipDistance( 0.2f );
            camera->setFarClipDistance( 1000.0f );
            camera->setAutoAspectRatio( true );

            if(SystemSettings::getUseDefaultLights()){
                sceneManager->setAmbientLight( Ogre::ColourValue( 0.3f, 0.3f, 0.3f ), Ogre::ColourValue( 0.02f, 0.53f, 0.96f ) * 0.1f, Ogre::Vector3::UNIT_Y );

                Ogre::Light *light = sceneManager->createLight();
                Ogre::SceneNode *lightNode = sceneManager->getRootSceneNode()->createChildSceneNode();
                lightNode->attachObject( light );
                light->setPowerScale( Ogre::Math::PI );
                light->setType( Ogre::Light::LT_DIRECTIONAL );
                light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );
            }

            sceneManager->setForward3D( true, 4,4,5,96,3,200 );

            *_sceneManager = sceneManager;
            *_camera = camera;


            //Queue 240 is used for rect2d and movable textures.
            sceneManager->getRenderQueue()->setRenderQueueMode(240, Ogre::RenderQueue::FAST);
            //For particles
            //sceneManager->getRenderQueue()->setRenderQueueMode(5,Ogre::RenderQueue::Modes::V1_FAST);
            //Set all of these to fast, just because they're user accessible.
            for(uint8 i = 0; i < 100; i++){
                sceneManager->getRenderQueue()->setRenderQueueMode(i, Ogre::RenderQueue::FAST);
            }

            //Register the movable factory.
            Rect2dMovableFactory* factory = new Rect2dMovableFactory(); //TODO delete this.
            Ogre::Root::getSingleton().addMovableObjectFactory(factory);
        }

        template<typename T>
        void setupSingleHLMS(Ogre::Root* root, const char* name){
            using namespace Ogre;

            Ogre::RenderSystem *renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();
            const std::string &rPath = SystemSettings::getMasterPath();

            // For retrieval of the paths to the different folders needed
            Ogre::String mainFolderPath;
            Ogre::StringVector libraryFoldersPaths;
            Ogre::StringVector::const_iterator libraryFolderPathIt;
            Ogre::StringVector::const_iterator libraryFolderPathEn;

            Ogre::ArchiveManager &archiveManager = Ogre::ArchiveManager::getSingleton();

            T *newHlms = 0;

            T::getDefaultPaths( mainFolderPath, libraryFoldersPaths );
            std::string templatePath = SystemSettings::getHlmsTemplatePath(name);
            std::string targetPath = rPath + mainFolderPath;
            if(!templatePath.empty()){
                targetPath = templatePath + "/" + mainFolderPath;
            }
            Archive *archivePbs = archiveManager.load( targetPath, "FileSystem", true );

            // Get the library archive(s)
            ArchiveVec archivePbsLibraryFolders;
            libraryFolderPathIt = libraryFoldersPaths.begin();
            libraryFolderPathEn = libraryFoldersPaths.end();
            while( libraryFolderPathIt != libraryFolderPathEn )
            {
                Archive *archiveLibrary =
                    archiveManager.load( rPath + *libraryFolderPathIt, "FileSystem", true );
                archivePbsLibraryFolders.push_back( archiveLibrary );
                ++libraryFolderPathIt;
            }
            _registerHlmsPieceLibraryToHlms(name, archivePbsLibraryFolders);

            // Create and register
            newHlms = OGRE_NEW T( archivePbs, &archivePbsLibraryFolders );
            Root::getSingleton().getHlmsManager()->registerHlms( newHlms );
        }

        void setupHLMS(Ogre::Root *root){
            setupSingleHLMS<Ogre::HlmsPbsAVCustom>(root, "pbs");
            setupSingleHLMS<Ogre::HlmsColibri>(root, "unlit");
            setupSingleHLMS<Ogre::HlmsTerra>(root, "terra");

            Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(false);
        }

        void setupCompositor(Ogre::Root *root, Ogre::SceneManager* sceneManager, Ogre::Camera* camera, Ogre::Window* window){
            if(!SystemSettings::getUseDefaultCompositor()) return;

            using namespace Ogre;

            CompositorManager2 *compositorManager = root->getCompositorManager2();

#if 0
            CompositorChannelVec externalChannels(2);
            externalChannels[0] = window->getTexture();

            //Terra's Shadow texture
            ResourceLayoutMap initialLayouts;
            ResourceAccessMap initialUavAccess;

            {
                //The texture is not available. Create a dummy dud using PF_NULL.
                TextureGpuManager *textureManager = root->getRenderSystem()->getTextureGpuManager();
                TextureGpu *nullTex = textureManager->createOrRetrieveTexture( "DummyNull",
                                                                               GpuPageOutStrategy::Discard,
                                                                               TextureFlags::ManualTexture,
                                                                               TextureTypes::Type2D );
                nullTex->setResolution( 1u, 1u );
                nullTex->setPixelFormat( PFG_R10G10B10A2_UNORM );
                nullTex->scheduleTransitionTo( GpuResidency::Resident );
                externalChannels[1] = nullTex;
                /*TexturePtr nullTex = TextureManager::getSingleton().createManual(
                            "DummyNull", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                            TEX_TYPE_2D, 1, 1, 0, PF_NULL );
                externalChannels[1].target = nullTex->getBuffer(0)->getRenderTarget();
                externalChannels[1].textures.push_back( nullTex );*/
            }

            {
                Ogre::CompositorNodeDef* nodeDef = compositorManager->getNodeDefinitionNonConst("Tutorial_TerrainRenderingNode");

                Ogre::CompositorTargetDef* targetDef = nodeDef->getTargetPass(0);
                Ogre::CompositorPassDef* def = targetDef->getCompositorPassesNonConst()[0];
                Ogre::CompositorPassClearDef* clearDef = static_cast<Ogre::CompositorPassClearDef*>(def);
                //clearDef->mColourValue = SystemSettings::getCompositorColourValue();
                clearDef->mClearColour[0] = SystemSettings::getCompositorColourValue();
            }

            Ogre::CompositorWorkspace* w = compositorManager->addWorkspace( sceneManager, externalChannels, camera,
                                                    "Tutorial_TerrainWorkspace", true, -1,
                                                    (UavBufferPackedVec*)0, &initialLayouts,
                                                    &initialUavAccess );

#endif




            CompositorChannelVec externalChannels(2);
            externalChannels[0] = window->getTexture();

            //Terra's Shadow texture
            //ResourceLayoutMap initialLayouts;
            //ResourceAccessMap initialUavAccess;

            ResourceStatusMap initialLayouts;

            if(false){
                //The texture is not available. Create a dummy dud using PF_NULL.
                TextureGpuManager *textureManager = root->getRenderSystem()->getTextureGpuManager();
                TextureGpu *nullTex = textureManager->createOrRetrieveTexture( "DummyNull",
                                                                               GpuPageOutStrategy::Discard,
                                                                               TextureFlags::ManualTexture,
                                                                               TextureTypes::Type2D );
                nullTex->setResolution( 1u, 1u );
                nullTex->setPixelFormat( PFG_R10G10B10A2_UNORM );
                nullTex->scheduleTransitionTo( GpuResidency::Resident );
                externalChannels[1] = nullTex;
                /*TexturePtr nullTex = TextureManager::getSingleton().createManual(
                            "DummyNull", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                            TEX_TYPE_2D, 1, 1, 0, PF_NULL );
                externalChannels[1].target = nullTex->getBuffer(0)->getRenderTarget();
                externalChannels[1].textures.push_back( nullTex );*/
            }

            {
                Ogre::CompositorNodeDef* nodeDef = compositorManager->getNodeDefinitionNonConst("Tutorial_TerrainRenderingNode");

                Ogre::CompositorTargetDef* targetDef = nodeDef->getTargetPass(0);
                Ogre::CompositorPassDef* def = targetDef->getCompositorPassesNonConst()[0];
                Ogre::CompositorPassClearDef* clearDef = static_cast<Ogre::CompositorPassClearDef*>(def);
                //clearDef->mColourValue = SystemSettings::getCompositorColourValue();
                clearDef->mClearColour[0] = SystemSettings::getCompositorColourValue();
            }

            /*Ogre::CompositorWorkspace* w = compositorManager->addWorkspace( sceneManager, externalChannels, camera,
                                                    "Tutorial_TerrainWorkspace", true, -1,
                                                    (UavBufferPackedVec*)0, &initialLayouts);
*/
            CompositorWorkspace *workspace = compositorManager->addWorkspace(
                sceneManager, window->getTexture(), camera, "Tutorial_TerrainWorkspace", true );





        }

    protected:
        void _setupResourcesFromFile(){

        }
    };
}
