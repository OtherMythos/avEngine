#pragma once

#include <string>

namespace Ogre{
    class Terra;
    class SceneManager;
    class Camera;
    class Vector3;
}

namespace AV{

    class TerrainObject{
    public:
        TerrainObject(Ogre::SceneManager* sceneManager, Ogre::Camera* camera);
        ~TerrainObject();

        void update();
        void load(const std::string& textureName, const Ogre::Vector3& origin, const Ogre::Vector3& size);

        Ogre::Terra* getTerra() { return mTerra; }

    private:
        Ogre::Terra* mTerra;
        Ogre::SceneManager* mSceneManager;

    };

}
