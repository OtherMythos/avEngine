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

        enum DrawAxis{
            AXIS_X, AXIS_Y, AXIS_Z
        };

        void drawPoint(const SlotPosition& first);
        void drawAxis(const SlotPosition& pos, DrawAxis axis);
        void drawCircle(const SlotPosition& pos, float radius);
        void drawSphere(const SlotPosition& pos, float radius);

    private:
        Ogre::SceneManager* mSceneManager;
        Ogre::SceneNode* mParentNode;

        enum ObjectType{
            Point,
            Axis,
            Circle,
            Sphere,
            Max
        };

        struct ObjectEntryData{
            size_t objectsUsed;
            std::vector<Ogre::SceneNode*> objects;
        };

        ObjectEntryData mData[ObjectType::Max];

        Ogre::SceneNode* _obtainDrawPoint(ObjectType type);
        const char* _getTypeMesh(ObjectType type) const;

        bool mShapeChanged = false;
    };
}

#endif
