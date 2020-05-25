#ifdef DEBUGGING_TOOLS

#include "MeshVisualiser.h"

#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreRoot.h"
#include "OgreHlmsUnlit.h"
#include "OgreHlmsUnlitDatablock.h"
#include "OgreHlmsManager.h"
#include "OgreItem.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"

namespace AV{
    const char* MeshVisualiser::mDatablockNames[MeshVisualiser::NUM_CATEGORIES] = {"internal/PhysicsChunk"};

    MeshVisualiser::MeshVisualiser(){
        EventDispatcher::subscribe(EventType::World, AV_BIND(MeshVisualiser::worldEventReceiver));
    }

    MeshVisualiser::~MeshVisualiser(){
        for(const auto& e : mAttachedPhysicsChunks){
            _destroyPhysicsChunk(e.second);
        }

        Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT);
        for(const char* d : mDatablockNames){
            hlms->destroyDatablock(d);
        }

        mAttachedPhysicsChunks.clear();

        EventDispatcher::unsubscribe(EventType::World, this);
    }

    void MeshVisualiser::initialise(Ogre::SceneManager* sceneManager){
        mSceneManager = sceneManager;

        mParentNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
        mPhysicsChunkNode = mParentNode->createChildSceneNode();

        Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT);
        Ogre::HlmsUnlit* unlit = dynamic_cast<Ogre::HlmsUnlit*>(hlms);

        for(int i = 0; i < NUM_CATEGORIES; i++){
            Ogre::HlmsDatablock* block = unlit->createDatablock(mDatablockNames[i], mDatablockNames[i], Ogre::HlmsMacroblock(), Ogre::HlmsBlendblock(), Ogre::HlmsParamVec(), false);
            Ogre::HlmsUnlitDatablock* unlitBlock = dynamic_cast<Ogre::HlmsUnlitDatablock*>(block);
            unlitBlock->setUseColour(true);
            //unlitBlock->setColour(Ogre::ColourValue(0.4, 0.4, 0.9));
            unlitBlock->setColour(Ogre::ColourValue(0, 0, 1));

            mCategoryDatablocks[i] = unlitBlock;
        }
    }

    void MeshVisualiser::setMeshGroupVisible(MeshGroupType type, bool visible){
        if((type & PhysicsChunk) == PhysicsChunk){
            mPhysicsChunkNode->setVisible(visible);
        }
    }

    void MeshVisualiser::insertPhysicsChunk(const PhysicsTypes::PhysicsChunkEntry& chunk){
        assert(chunk.first && chunk.second);
        assert(mAttachedPhysicsChunks.find(chunk) == mAttachedPhysicsChunks.end());

        Ogre::SceneNode* chunkNode = mPhysicsChunkNode->createChildSceneNode();

        for(const btRigidBody* b : *(chunk.second) ){
            Ogre::SceneNode* bodyNode = chunkNode->createChildSceneNode();
            const btVector3& pos = b->getWorldTransform().getOrigin();
            bodyNode->setPosition( Ogre::Vector3(pos.x(), pos.y(), pos.z()) );

            const char* meshObject = 0;
            const btCollisionShape* shape = b->getCollisionShape();
            Ogre::Vector3 posScale;

            int shapeType = shape->getShapeType();
            if(shapeType == BOX_SHAPE_PROXYTYPE){
                meshObject = "lineBox";

                btVector3 scaleAmount = ((btBoxShape*)shape)->getHalfExtentsWithoutMargin();
                posScale = Ogre::Vector3(scaleAmount.x(), scaleAmount.y(), scaleAmount.z());
            }else {
                assert(false);
            }

            bodyNode->setScale(posScale);
            Ogre::Item *item = mSceneManager->createItem(meshObject, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_DYNAMIC);
            item->setDatablock(mCategoryDatablocks[0]);
            bodyNode->attachObject((Ogre::MovableObject*)item);
        }
        mAttachedPhysicsChunks[chunk] = chunkNode;
    }

    void MeshVisualiser::destroyPhysicsChunk(const PhysicsTypes::PhysicsChunkEntry& chunk){
        auto it = mAttachedPhysicsChunks.find(chunk);
        assert(it != mAttachedPhysicsChunks.end());

        Ogre::SceneNode* node = (*it).second;
        assert(node);

        _destroyPhysicsChunk(node);
        mAttachedPhysicsChunks.erase(it);
    }

    void MeshVisualiser::_destroyPhysicsChunk(Ogre::SceneNode* node){
        _recursiveDestroyNode(node);
    }

    //TODO This is a useful function. I might want to move it somewhere else for convenience.
    void MeshVisualiser::_recursiveDestroyNode(Ogre::SceneNode* node){

        auto it = node->getChildIterator();
        while(it.current() != it.end()){
            Ogre::SceneNode *n = (Ogre::SceneNode*)it.getNext();
            _recursiveDestroyNode(n);
            _destroyMovableObject(n);

            mSceneManager->destroySceneNode(n);
        }
    }

    void MeshVisualiser::_destroyMovableObject(Ogre::SceneNode* node){
        //When it comes time to destroy a mesh (movable object) a bit more work needs to be done, as you have to check it's actually there.
        Ogre::SceneNode::ObjectIterator it = node->getAttachedObjectIterator();
        if(it.hasMoreElements()){
            Ogre::MovableObject* obj = it.getNext();
            mSceneManager->destroyMovableObject(obj);
        }
        //At the moment a mesh object should only have the one mesh attached to it, so by this point there should be no more.
        assert(!it.hasMoreElements());
    }

    void MeshVisualiser::_repositionMeshesOriginShift(const Ogre::Vector3& offset){
        auto it = mParentNode->getChildIterator();
        while(it.current() != it.end()){
            Ogre::SceneNode *node = (Ogre::SceneNode*)it.getNext();

            node->setPosition(node->getPosition() - offset);
        }
    }

    bool MeshVisualiser::worldEventReceiver(const Event &e){
        const WorldEvent& event = (WorldEvent&)e;
        if(event.eventCategory() == WorldEventCategory::OriginChange){
            const WorldEventOriginChange& originEvent = (WorldEventOriginChange&)event;

            _repositionMeshesOriginShift(originEvent.worldOffset);
        }
        return true;
    }
}

#endif
