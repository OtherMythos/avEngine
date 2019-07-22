#pragma once

namespace Ogre{
    class Terra;
}

namespace AV{
    class Terrain{
    public:
        Terrain();
        ~Terrain();

        void update();

    private:
        Ogre::Terra* mTerra;
    };
}
