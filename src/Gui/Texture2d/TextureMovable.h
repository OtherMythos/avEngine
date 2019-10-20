#pragma once

#include "OgreMovableObject.h"

namespace AV{
    class Rect2dRenderable;

    class TextureMovable : public Ogre::MovableObject{
    public:
       TextureMovable(Ogre::IdType id,
             Ogre::ObjectMemoryManager* objectMemoryManager,
             Ogre::SceneManager* sceneManager, Ogre::uint8 renderQueueId);
       ~TextureMovable();

       void attachRect2dRenderable(Rect2dRenderable* renderable);
       void detachRect2dRenderable(Rect2dRenderable* renderable);

       const Ogre::String& getMovableType() const;

    private:
        Rect2dRenderable* renderable;
    };



    class TextureMovableFactory : public Ogre::MovableObjectFactory
    {
       public:
          TextureMovableFactory(){};
          ~TextureMovableFactory(){};

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
