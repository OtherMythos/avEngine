#include "TextureMovable.h"

#include "Rect2dRenderable.h"

#include "Ogre.h"

namespace AV{

    Ogre::String TextureMovableFactory::FACTORY_TYPE_NAME = "TextureMovable";

    TextureMovable::TextureMovable(Ogre::IdType id,
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

    TextureMovable::~TextureMovable(){
    }

    void TextureMovable::attachRect2dRenderable(Rect2dRenderable* renderable){
       assert(this->renderable == NULL);

       mRenderables.push_back(renderable);
       this->renderable = renderable;
    }

    void TextureMovable::detachRect2dRenderable(Rect2dRenderable* renderable){
       assert(this->renderable == renderable);
       mRenderables.pop_back();
       this->renderable = 0;
    }

    const Ogre::String& TextureMovable::getMovableType() const{
       return TextureMovableFactory::FACTORY_TYPE_NAME;
    }


    //Factory

    Ogre::MovableObject* TextureMovableFactory::createInstanceImpl(
          Ogre::IdType id, Ogre::ObjectMemoryManager* objectMemoryManager,
          Ogre::SceneManager* manager, const Ogre::NameValuePairList* params)
    {
        Rect2dRenderable* renderable = OGRE_NEW Rect2dRenderable();

        renderable->setDatablock("HlmsUnlit1"); //Here for now.

        TextureMovable* movable = OGRE_NEW TextureMovable(id, objectMemoryManager, manager,
             240); //240 is the defined render queue for textures. If I want to dress it up I would move it into a define.

        movable->attachRect2dRenderable(renderable);

        return movable;
    }

    void TextureMovableFactory::destroyInstance(Ogre::MovableObject* obj){
        TextureMovable* mov = static_cast<TextureMovable*>(obj);

        assert(obj->mRenderables.size() == 1); //There should always be one renderable.
        Rect2dRenderable* rend = static_cast<Rect2dRenderable*>(obj->mRenderables[0]);

        mov->detachRect2dRenderable(rend);
        OGRE_DELETE rend;

        OGRE_DELETE obj;
    }

    const Ogre::String& TextureMovableFactory::getType(void) const
    {
       return FACTORY_TYPE_NAME;
    }
}
