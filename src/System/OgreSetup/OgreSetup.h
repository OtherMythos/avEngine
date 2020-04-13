#pragma once

#include "Ogre.h"
#include "OgreWindow.h"
#include "OgreTextureGpuManager.h"

#include "System/SystemSetup/SystemSettings.h"
#include "filesystem/path.h"

#include "Compositor/OgreCompositorWorkspace.h"
#include <Compositor/OgreCompositorManager2.h>
#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorNodeDef.h"
#include "Compositor/Pass/PassClear/OgreCompositorPassClearDef.h"

#include "Gui/Rect2d/Compositor/CompositorPassRect2dProvider.h"

#include "Gui/Rect2d/Rect2dMovable.h"

namespace Ogre {
    class Root;
}

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
        virtual void setupHLMS(Ogre::Root *root) = 0;
        virtual void setupOgreWindow(Window *window) = 0;
        //virtual void setupScene(Ogre::Root *root, Ogre::SceneManager **sceneManager, Ogre::Camera **camera) = 0;

        void setupOgreResources(Ogre::Root *root){

            //TODO separate these have to be added locations from the others with a different function.
            { //Process the essential files. These are processed regardless of whether an ogre resources file was found, as the engine assumes they exist.
                const Ogre::String& masterPath = SystemSettings::getMasterPath();

                //Add the essential files to the resource locations. This includes things like compositor files, shaders and so on.
                root->addResourceLocation(masterPath + "/essential/terrain", "FileSystem", "General");
                root->addResourceLocation(masterPath + "/essential/terrain/GLSL", "FileSystem", "General");
                //TODO make it so HLSL isn't included in a linux build, and the same for the other platforms.
                root->addResourceLocation(masterPath + "/essential/terrain/HLSL", "FileSystem", "General");
                root->addResourceLocation(masterPath + "/essential/terrain/Metal", "FileSystem", "General");
                root->addResourceLocation(masterPath + "/essential/common", "FileSystem", "General");

                root->addResourceLocation(masterPath + "/essential/common/Any", "FileSystem", "General");
                root->addResourceLocation(masterPath + "/essential/common/GLSL", "FileSystem", "General");
                root->addResourceLocation(masterPath + "/essential/common/HLSL", "FileSystem", "General");
                root->addResourceLocation(masterPath + "/essential/common/Metal", "FileSystem", "General");

                root->addResourceLocation(masterPath + "/essential/compositor", "FileSystem", "General");

                //Temporary
                #ifdef __linux__
                    root->addResourceLocation("/home/edward/Documents/avDeps/colibrigui/bin/Data/Materials/ColibriGui/Skins/Debug", "FileSystem", "General");
                #elif _WIN32
                    root->addResourceLocation("C:\\Users\\edward\\Documents\\avDeps\\colibrigui\\bin\\Data\\Materials\\ColibriGui\\Skins\\DarkGloss", "FileSystem", "General");
                #endif
            }


            if(!SystemSettings::isOgreResourcesFileViable()) return;

            Ogre::ConfigFile cf;
            cf.load(SystemSettings::getOgreResourceFilePath());

            Ogre::String name, locType;
            Ogre::ConfigFile::SectionIterator secIt = cf.getSectionIterator();

            filesystem::path relativePath(SystemSettings::getOgreResourceFilePath());
            relativePath = relativePath.parent_path();

            while (secIt.hasMoreElements()){
                Ogre::ConfigFile::SettingsMultiMap* settings = secIt.getNext();
                Ogre::ConfigFile::SettingsMultiMap::iterator it;

                for (it = settings->begin(); it != settings->end(); ++it){
                    locType = it->first;
                    name = it->second;

                    filesystem::path valuePath(name);
                    filesystem::path resolvedPath;
                    if(!valuePath.is_absolute()){
                        //If an absolute path was not provided determine an absolute.
                        resolvedPath = relativePath / valuePath;
                        if(!resolvedPath.exists() || !resolvedPath.is_directory()) continue;

                        resolvedPath = resolvedPath.make_absolute();
                    }else{
                        resolvedPath = valuePath;
                        if(!resolvedPath.exists() || !resolvedPath.is_directory()) continue;
                    }

                    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(resolvedPath.str(), locType);
                    AV_INFO("Adding {} to {}", resolvedPath.str(), locType);
                }
            }
        }

        void setupScene(Ogre::Root *root, Ogre::SceneManager **_sceneManager, Ogre::Camera **_camera){
            Ogre::SceneManager *sceneManager = root->createSceneManager(Ogre::ST_GENERIC, 2, "Scene Manager");

            Ogre::Camera *camera = sceneManager->createCamera("Main Camera");

            //camera->setPosition(Ogre::Vector3( 0, 0, 100 ));  //camera->setPosition( Ogre::Vector3( 0, 50, -100 ) );
            // Look back along -Z
            //camera->lookAt( Ogre::Vector3( 0, 0, 0 ) );
            camera->setNearClipDistance( 0.2f );
            camera->setFarClipDistance( 1000.0f );
            camera->setAutoAspectRatio( true );

            sceneManager->setAmbientLight( Ogre::ColourValue( 0.3f, 0.3f, 0.3f ), Ogre::ColourValue( 0.02f, 0.53f, 0.96f ) * 0.1f, Ogre::Vector3::UNIT_Y );

            Ogre::Light *light = sceneManager->createLight();
            Ogre::SceneNode *lightNode = sceneManager->getRootSceneNode()->createChildSceneNode();
            lightNode->attachObject( light );
            light->setPowerScale( Ogre::Math::PI );
            light->setType( Ogre::Light::LT_DIRECTIONAL );
            light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );

            sceneManager->setForward3D( true, 4,4,5,96,3,200 );

//            Ogre::SceneNode *node = sceneManager->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_DYNAMIC);
//            Ogre::Item *item = sceneManager->createItem("ogrehead2.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_DYNAMIC);
//            node->attachObject((Ogre::MovableObject*)item);

            *_sceneManager = sceneManager;
            *_camera = camera;



            sceneManager->getRenderQueue()->setRenderQueueMode(240, Ogre::RenderQueue::FAST);

            //Register the movable factory.
            Rect2dMovableFactory* factory = new Rect2dMovableFactory(); //TODO delete this.
            Ogre::Root::getSingleton().addMovableObjectFactory(factory);
        }

        void setupCompositor(Ogre::Root *root, Ogre::SceneManager* sceneManager, Ogre::Camera* camera, Ogre::Window* window){
            using namespace Ogre;

            CompositorManager2 *compositorManager = root->getCompositorManager2();

            //CompositorWorkspace *oldWorkspace = mGraphicsSystem->getCompositorWorkspace();
            CompositorWorkspace *oldWorkspace = 0;
            /*if( oldWorkspace )
            {
                TexturePtr terraShadowTex = oldWorkspace->getExternalRenderTargets()[1].textures.back();
                if( terraShadowTex->getFormat() == PF_NULL )
                {
                    ResourcePtr resourcePtr( terraShadowTex );
                    TextureManager::getSingleton().remove( resourcePtr );
                }
                compositorManager->removeWorkspace( oldWorkspace );
            }*/

            CompositorChannelVec externalChannels( 2 );
            //Render window
            //externalChannels[0].target = window;
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

        }

    protected:
        void _setupResourcesFromFile(){

        }
    };
}
