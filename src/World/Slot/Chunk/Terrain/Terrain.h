#pragma once

namespace Ogre{
    class Terra;
    class SceneNode;
}

namespace AV{
    class Terrain{
    public:
        Terrain(Ogre::SceneNode* n);
        ~Terrain();

        void update();

    private:
        Ogre::Terra* mTerra;
    };
}
