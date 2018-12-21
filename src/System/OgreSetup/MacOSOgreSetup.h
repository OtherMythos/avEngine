#pragma once

#include "OgreSetup.h"
#include "System/ResourcePathContainer.h"

#include "Ogre.h"
#include <OgreHlmsPbs.h>
#include <OgreHlmsUnlit.h>
#include "Logger/Log.h"
#include <Compositor/OgreCompositorManager2.h>


#include <OgreMetalPlugin.h>
//#include <OgreGL3PlusPlugin.h>

namespace AV {
    /**
     An implementation of the ogre setup for MacOS.
     */
    class MacOSOgreSetup : public OgreSetup{
    public:
        MacOSOgreSetup() {}
        
        Ogre::Root* setupRoot(){
            Ogre::Root *root = new Ogre::Root();
            
            root->loadPlugin("RenderSystem_Metal");
            //root->loadPlugin("RenderSystem_GL3Plus");
            
            //root->installPlugin(new Ogre::MetalPlugin());
            //root->installPlugin(new Ogre::GL3PlusPlugin());
            root->setRenderSystem(root->getAvailableRenderers()[0]);
            root->getRenderSystem()->setConfigOption( "sRGB Gamma Conversion", "Yes" );
            root->initialise(false);
            
            root->addResourceLocation("/Users/edward/Documents/mesh", "FileSystem");
            Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(false);
            
            return root;
        }
        
        void setupOgreWindow(Window *window){
            SDL2Window *sdlWindow = (SDL2Window*)window;
            
            Ogre::NameValuePairList params;
            
            params.insert( std::make_pair("macAPI", "cocoa") );
            params.insert( std::make_pair("macAPICocoaUseNSView", "true") );
            params["parentWindowHandle"] = Ogre::StringConverter::toString(sdlWindow->getHandle());
            
            Ogre::RenderWindow *renderWindow = Ogre::Root::getSingleton().createRenderWindow("Ogre Window", 500, 400, false, &params);
            renderWindow->setVisible(true);
            
            sdlWindow->injectOgreWindow(renderWindow);
        }
        
        void setupHLMS(Ogre::Root *root){
            Ogre::RenderSystem *renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();
            
            Ogre::ArchiveVec library;
            const std::string &rPath = ResourcePathContainer::getResourcePath();
            
            if(renderSystem->getName() == "Metal Rendering Subsystem"){
                library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Common/Metal", "FileSystem", true ));
            }else if(renderSystem->getName() == "OpenGL 3+ Rendering Subsystem"){
                library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Common/GLSL", "FileSystem", true ));
            }
            
            library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Unlit/Any", "FileSystem", true ));
            library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/Any", "FileSystem", true ));
            //library.push_back(Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Common/Any", "FileSystem", true ));
            
            Ogre::Archive *archivePbs;
            Ogre::Archive *archiveUnlit;
            
            if(renderSystem->getName() == "Metal Rendering Subsystem"){
                AV_INFO("Wow Metal");
                archivePbs = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/Metal", "FileSystem", true );
                archiveUnlit = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Unlit/Metal", "FileSystem", true );
            }else if(renderSystem->getName() == "OpenGL 3+ Rendering Subsystem"){
                archivePbs = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Pbs/GLSL", "FileSystem", true );
                archiveUnlit = Ogre::ArchiveManager::getSingletonPtr()->load(rPath + "/Hlms/Unlit/GLSL", "FileSystem", true );
            }
            Ogre::HlmsPbs *hlmsPbs = new Ogre::HlmsPbs( archivePbs, &library );
            Ogre::HlmsUnlit *hlmsUnlit = new Ogre::HlmsUnlit( archiveUnlit, &library );
            
            root->getHlmsManager()->registerHlms(hlmsPbs);
            root->getHlmsManager()->registerHlms(hlmsUnlit);
        }
        
        void setupCompositor(Ogre::Root *root, Ogre::SceneManager* sceneManager, Ogre::Camera *camera, Ogre::RenderWindow *window){
            Ogre::CompositorManager2 *compositorManager = root->getCompositorManager2();
            
            const Ogre::String workspaceName("test Workspace");
            if(!compositorManager->hasWorkspaceDefinition(workspaceName)){
                compositorManager->createBasicWorkspaceDef(workspaceName, Ogre::ColourValue(1, 0, 0, 1));
            }
            
            compositorManager->addWorkspace(sceneManager, window, camera, workspaceName, true);
        }
        
        void setupScene(Ogre::Root *root, Ogre::SceneManager **_sceneManager, Ogre::Camera **_camera){
            Ogre::SceneManager *sceneManager = root->createSceneManager(Ogre::ST_GENERIC, 2, Ogre::INSTANCING_CULLING_SINGLETHREAD, "Scene Manager");
            
            Ogre::Camera *camera = sceneManager->createCamera("Main Camera");
            
            camera->setPosition(Ogre::Vector3( 0, 0, 100 ));  //camera->setPosition( Ogre::Vector3( 0, 50, -100 ) );
            // Look back along -Z
            camera->lookAt( Ogre::Vector3( 0, 0, 0 ) );
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
            
            Ogre::SceneNode *node = sceneManager->getRootSceneNode()->createChildSceneNode(Ogre::SCENE_DYNAMIC);
            Ogre::Item *item = sceneManager->createItem("ogrehead2.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_DYNAMIC);
            node->attachObject((Ogre::MovableObject*)item);
            
            *_sceneManager = sceneManager;
            *_camera = camera;
        }
    };
}
