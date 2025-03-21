#pragma once

namespace Ogre{
    class Terra;
    class SceneManager;
    class Camera;
}

namespace AV{

    class TerrainObject{
    public:
        TerrainObject(Ogre::SceneManager* sceneManager, Ogre::Camera* camera);
        ~TerrainObject();

        void update();

    private:
        Ogre::Terra* mTerra;
        Ogre::SceneManager* mSceneManager;

    };

}
