#ifdef DEBUGGING_TOOLS

#pragma once

#include <vector>

namespace Ogre{
    class SceneManager;
    class SceneNode;
}

namespace AV{
    class SlotPosition;

    class DebugDrawer{
    public:
        DebugDrawer();
        ~DebugDrawer();

        void initialise(Ogre::SceneManager* sceneManager);

        void resetDraw();

        void drawPoint(const SlotPosition& first);

    private:
        Ogre::SceneManager* mSceneManager;
        Ogre::SceneNode* mParentNode;

        std::vector<Ogre::SceneNode*> mPointObjects;
        size_t mPointObjectsUsed = 0;

        Ogre::SceneNode* _obtainDrawPoint();

        bool mShapeChanged = false;
    };
}

#endif
