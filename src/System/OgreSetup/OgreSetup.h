#pragma once

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
        virtual void setupOgreResources(Ogre::Root *root) = 0;
        virtual void setupCompositor(Ogre::Root *root, Ogre::SceneManager* sceneManager, Ogre::Camera *camera, Ogre::RenderWindow *window) = 0;
        virtual void setupOgreWindow(Window *window) = 0;
        virtual void setupScene(Ogre::Root *root, Ogre::SceneManager **sceneManager, Ogre::Camera **camera) = 0;
        
    protected:
        void _setupResourcesFromFile(){
            
        }
    };
}
