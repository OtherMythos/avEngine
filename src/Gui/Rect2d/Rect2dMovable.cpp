#include "Rect2dMovable.h"

#include "Rect2dRenderable.h"

#include "Ogre.h"

namespace AV{

    Ogre::String Rect2dMovableFactory::FACTORY_TYPE_NAME = "Rect2dMovable";

    Rect2dMovable::Rect2dMovable(Ogre::IdType id,
                          Ogre::ObjectMemoryManager* objectMemoryManager,
                          Ogre::SceneManager* sceneManager,
                          Ogre::uint8 renderQueueId)
                       : Ogre::MovableObject(id, objectMemoryManager, sceneManager,
                                      renderQueueId)
    {
       this->renderable = NULL;

       //The bounding box needs to be infinite so no clipping tests are failed.
       mObjectData.mLocalAabb->setFromAabb(Ogre::Aabb::BOX_INFINITE,
             mObjectData.mIndex);
       mObjectData.mWorldAabb->setFromAabb(Ogre::Aabb::BOX_INFINITE,
             mObjectData.mIndex );
       mObjectData.mLocalRadius[mObjectData.mIndex] =
          Ogre::Aabb::BOX_INFINITE.getRadius();
       mObjectData.mWorldRadius[mObjectData.mIndex] =
          Ogre::Aabb::BOX_INFINITE.getRadius();
       mObjectData.mQueryFlags[mObjectData.mIndex] =
          Ogre::SceneManager::QUERY_ENTITY_DEFAULT_MASK;
    }

    Rect2dMovable::~Rect2dMovable(){
    }

    void Rect2dMovable::attachRect2dRenderable(Rect2dRenderable* renderable){
       assert(this->renderable == NULL);

       mRenderables.push_back(renderable);
       this->renderable = renderable;
    }

    void Rect2dMovable::detachRect2dRenderable(Rect2dRenderable* renderable){
       assert(this->renderable == renderable);
       mRenderables.pop_back();
       this->renderable = 0;
    }

    const Ogre::String& Rect2dMovable::getMovableType() const{
       return Rect2dMovableFactory::FACTORY_TYPE_NAME;
    }

    void Rect2dMovable::setDatablock(const std::string datablockName){
        renderable->setDatablock(datablockName);
    }


    //Factory

    Ogre::MovableObject* Rect2dMovableFactory::createInstanceImpl(
          Ogre::IdType id, Ogre::ObjectMemoryManager* objectMemoryManager,
          Ogre::SceneManager* manager, const Ogre::NameValuePairList* params)
    {
        Rect2dRenderable* renderable = OGRE_NEW Rect2dRenderable();

        Rect2dMovable* movable = OGRE_NEW Rect2dMovable(id, objectMemoryManager, manager,
             240); //240 is the defined render queue for textures. If I want to dress it up I would move it into a define.

        movable->attachRect2dRenderable(renderable);

        return movable;
    }

    void Rect2dMovableFactory::destroyInstance(Ogre::MovableObject* obj){
        Rect2dMovable* mov = static_cast<Rect2dMovable*>(obj);

        assert(obj->mRenderables.size() == 1); //There should always be one renderable.
        Rect2dRenderable* rend = static_cast<Rect2dRenderable*>(obj->mRenderables[0]);

        mov->detachRect2dRenderable(rend);
        OGRE_DELETE rend;

        OGRE_DELETE obj;
    }

    const Ogre::String& Rect2dMovableFactory::getType(void) const
    {
       return FACTORY_TYPE_NAME;
    }
}
