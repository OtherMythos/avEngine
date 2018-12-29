#pragma once

#include "Ogre.h"
#include "System/SystemSetup/SystemSettings.h"

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
        virtual void setupCompositor(Ogre::Root *root, Ogre::SceneManager* sceneManager, Ogre::Camera *camera, Ogre::RenderWindow *window) = 0;
        virtual void setupOgreWindow(Window *window) = 0;
        //virtual void setupScene(Ogre::Root *root, Ogre::SceneManager **sceneManager, Ogre::Camera **camera) = 0;

        void setupOgreResources(Ogre::Root *root){
            if(!SystemSettings::isOgreResourcesFileViable()) return;

            const std::string &rPath = SystemSettings::getDataPath();
            Ogre::ConfigFile cf;
            cf.load(SystemSettings::getOgreResourceFilePath());

            Ogre::String name, locType;
            Ogre::ConfigFile::SectionIterator secIt = cf.getSectionIterator();

            while (secIt.hasMoreElements()){
                Ogre::ConfigFile::SettingsMultiMap* settings = secIt.getNext();
                Ogre::ConfigFile::SettingsMultiMap::iterator it;

                for (it = settings->begin(); it != settings->end(); ++it){
                    locType = it->first;
                    name = it->second;

                    std::string totalPath = rPath + "/" + name;
                    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(totalPath, locType);
                    AV_INFO("Adding {} to {}", totalPath, locType);
                }
            }

            Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(false);

        }

        void setupScene(Ogre::Root *root, Ogre::SceneManager **_sceneManager, Ogre::Camera **_camera){
            Ogre::SceneManager *sceneManager = root->createSceneManager(Ogre::ST_GENERIC, 2, Ogre::INSTANCING_CULLING_SINGLETHREAD, "Scene Manager");

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
        }

    protected:
        void _setupResourcesFromFile(){

        }
    };
}
