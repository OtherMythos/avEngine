#pragma once

#include "OgreMovableObject.h"

namespace AV{
    class Rect2dRenderable;

    class Rect2dMovable : public Ogre::MovableObject{
    public:
       Rect2dMovable(Ogre::IdType id,
             Ogre::ObjectMemoryManager* objectMemoryManager,
             Ogre::SceneManager* sceneManager, Ogre::uint8 renderQueueId);
       ~Rect2dMovable();

       void attachRect2dRenderable(Rect2dRenderable* renderable);
       void detachRect2dRenderable(Rect2dRenderable* renderable);

       const Ogre::String& getMovableType() const;

       //Calls setDatablock onto the rect2drenderable.
       void setDatablock(const std::string datablockName);

    private:
        Rect2dRenderable* renderable;
    };



    class Rect2dMovableFactory : public Ogre::MovableObjectFactory
    {
       public:
          Rect2dMovableFactory(){};
          ~Rect2dMovableFactory(){};

          static Ogre::String FACTORY_TYPE_NAME;

          const Ogre::String& getType() const;

          void destroyInstance(Ogre::MovableObject* obj);

       protected:
          virtual Ogre::MovableObject* createInstanceImpl(Ogre::IdType id,
                Ogre::ObjectMemoryManager* objectMemoryManager,
                Ogre::SceneManager* manager,
                const Ogre::NameValuePairList* params = 0);
    };
}
