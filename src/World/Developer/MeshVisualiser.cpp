#ifdef DEBUGGING_TOOLS

#include "MeshVisualiser.h"

#include "OgreQuaternion.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreRoot.h"
#include "OgreHlmsUnlit.h"
#include "OgreHlmsUnlitDatablock.h"
#include "OgreHlmsManager.h"
#include "OgreItem.h"

#include "Event/EventDispatcher.h"
#include "Event/Events/WorldEvent.h"

#include "World/Physics/Worlds/CollisionWorldUtils.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"

#include "System/Util/OgreNodeHelper.h"
#include "Nav/NavMeshDebugDraw.h"
#include "Event/Events/DebuggerToolEvent.h"

#include "Logger/Log.h"

namespace AV{
    const char* MeshVisualiser::mDatablockNames[MeshVisualiser::NUM_CATEGORIES] = {
        "internal/DynamicsPhysicsChunk",
        "internal/Collision0",
        "internal/Collision1",
        "internal/Collision2",
        "internal/Collision3",
    };

    bool MeshVisualiser::debuggerToolsReceiver(const Event &e){
        const DebuggerToolEvent& event = (DebuggerToolEvent&)e;
        if(event.eventId() == EventId::DebuggingToolToggle){
            const DebuggerToolEventToggle& toolEvent = (DebuggerToolEventToggle&)event;

            if(toolEvent.t == DebuggerToolToggle::MeshesToggle){
                setOverrideVisible(!mVisibleOverride);
            }
        }

        return false;
    }

    MeshVisualiser::MeshVisualiser()
        : mNavMeshDebugDraw(std::make_shared<NavMeshDebugDraw>()),
        mVisibleOverride(true) {
        EventDispatcher::subscribe(EventType::World, AV_BIND(MeshVisualiser::worldEventReceiver));
        EventDispatcher::subscribe(EventType::DebuggerTools, AV_BIND(MeshVisualiser::debuggerToolsReceiver));

        for(int i = 0; i < MAX_COLLISION_WORLDS; i++)
            mCollisionWorldObjectNodes[i] = 0;
    }

    MeshVisualiser::~MeshVisualiser(){
        for(const auto& e : mAttachedPhysicsChunks){
            OgreNodeHelper::destroyNodeAndChildren(e.second);
        }
        for(const auto& e : mAttachedCollisionObjectChunks){
            OgreNodeHelper::destroyNodeAndChildren(e.second);
        }
        for(const auto& e : mAttachedNavMeshes){
            OgreNodeHelper::destroyNodeAndChildren(e.second);
        }

        //This destruction happens during a complete shutdown, so it's not a problem to completely wipe the list.

        //Destroy parent nodes. Their children should already be destroyed.
        mSceneManager->destroySceneNode(mPhysicsChunkNode);
        mSceneManager->destroySceneNode(mCollisionObjectsChunkNode);
        mSceneManager->destroySceneNode(mNavMeshObjectNode);

        for(int i = 0; i < MAX_COLLISION_WORLDS; i++){
            if(!mCollisionWorldObjectNodes[i]) continue;
            OgreNodeHelper::recursiveDestroyNode(mCollisionWorldObjectNodes[i]);
            mSceneManager->destroySceneNode(mCollisionWorldObjectNodes[i]);
        }

        //assert(mAttachedCollisionObjects.empty());
        mAttachedPhysicsChunks.clear();
        mAttachedCollisionObjects.clear();

        Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT);
        for(const char* d : mDatablockNames){
            hlms->destroyDatablock(d);
        }
        hlms->destroyDatablock(mNavMeshDatablock->getName());

        EventDispatcher::unsubscribe(EventType::World, this);
        EventDispatcher::unsubscribe(EventType::DebuggerTools, this);
    }

    void MeshVisualiser::initialise(Ogre::SceneManager* sceneManager){
        mSceneManager = sceneManager;

        mParentNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
        mPhysicsChunkNode = mParentNode->createChildSceneNode();
        mCollisionObjectsChunkNode = mParentNode->createChildSceneNode();
        mNavMeshObjectNode = mParentNode->createChildSceneNode();
        for(int i = 0; i < MAX_COLLISION_WORLDS; i++){
            mCollisionWorldObjectNodes[i] = mParentNode->createChildSceneNode();
        }

        Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT);
        Ogre::HlmsUnlit* unlit = dynamic_cast<Ogre::HlmsUnlit*>(hlms);

        static const Ogre::ColourValue datablockColours[NUM_CATEGORIES] = {
            Ogre::ColourValue(0, 0, 1),
            Ogre::ColourValue(0.2, 0.2, 1), //Light blue
            Ogre::ColourValue(1, 0.2, 0.2), //Light red
            Ogre::ColourValue(0.964, 0.419, 1), //Purple
            Ogre::ColourValue(1, 0.952, 0.419) //Yellow
        };

        for(int i = 0; i < NUM_CATEGORIES; i++){
            Ogre::HlmsDatablock* block = unlit->createDatablock(mDatablockNames[i], mDatablockNames[i], Ogre::HlmsMacroblock(), Ogre::HlmsBlendblock(), Ogre::HlmsParamVec(), false);
            Ogre::HlmsUnlitDatablock* unlitBlock = dynamic_cast<Ogre::HlmsUnlitDatablock*>(block);
            unlitBlock->setUseColour(true);
            unlitBlock->setColour(datablockColours[i]);

            mCategoryDatablocks[i] = unlitBlock;
        }

        {
            //Create the nav mesh datablock.
            Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT);
            Ogre::HlmsUnlit* unlit = dynamic_cast<Ogre::HlmsUnlit*>(hlms);
            Ogre::HlmsMacroblock mm;
            mm.mDepthCheck = true;
            Ogre::HlmsBlendblock bb;
            bb.setBlendType(Ogre::SceneBlendType::SBT_TRANSPARENT_ALPHA);

            const char* dbName = "NavMeshDebugDrawDatablock";
            mNavMeshDatablock = dynamic_cast<Ogre::HlmsUnlitDatablock*>(unlit->createDatablock(dbName, dbName, mm, bb, Ogre::HlmsParamVec()));
            Ogre::HlmsUnlitDatablock* unlitBlock = dynamic_cast<Ogre::HlmsUnlitDatablock*>(mNavMeshDatablock);
            unlitBlock->setUseColour(true);
        }
    }

    void MeshVisualiser::setMeshGroupVisible(MeshGroupType type, bool visible){
        if((type & PhysicsChunk) == PhysicsChunk){
            mPhysicsChunkNode->setVisible(visible);
        }
    }

    void MeshVisualiser::setOverrideVisible(bool visible){
        mVisibleOverride = visible;
        mParentNode->setVisible(mVisibleOverride);
    }

    void MeshVisualiser::insertCollisionObject(uint8 collisionWorldId, const btCollisionObject* obj){
        assert(collisionWorldId < MAX_COLLISION_WORLDS);
        Ogre::SceneNode* newNode = _createSceneNode(mCollisionWorldObjectNodes[collisionWorldId], obj, collisionWorldId + 1);
        mAttachedCollisionObjects[obj] = newNode;
    }

    void MeshVisualiser::removeCollisionObject(uint8 collisionWorldId, const btCollisionObject* obj){
        auto it = mAttachedCollisionObjects.find(obj);
        if(it == mAttachedCollisionObjects.end()) return;

        Ogre::SceneNode* targetNode = (*it).second;
        assert(targetNode);
        OgreNodeHelper::destroyMovableObject(targetNode);
        mSceneManager->destroySceneNode(targetNode);

        mAttachedCollisionObjects.erase(it);
    }

    Ogre::SceneNode* MeshVisualiser::_createSceneNode(Ogre::SceneNode* parent, const btCollisionObject* obj, uint8 datablockId){
        Ogre::SceneNode* bodyNode = parent->createChildSceneNode();
        const btVector3& pos = obj->getWorldTransform().getOrigin();
        bodyNode->setPosition( Ogre::Vector3(pos.x(), pos.y(), pos.z()) );
        const btQuaternion& orientation = obj->getWorldTransform().getRotation();
        bodyNode->setOrientation( Ogre::Quaternion(orientation.w(), orientation.x(), orientation.y(), orientation.z()) );

        const char* meshObject = 0;
        const btCollisionShape* shape = obj->getCollisionShape();
        Ogre::Vector3 posScale;

        int shapeType = shape->getShapeType();

        if(shapeType == BOX_SHAPE_PROXYTYPE){
            meshObject = "lineBox";

            btVector3 scaleAmount = ((btBoxShape*)shape)->getHalfExtentsWithoutMargin();
            posScale = Ogre::Vector3(scaleAmount.x(), scaleAmount.y(), scaleAmount.z());
        }else if(shapeType == SPHERE_SHAPE_PROXYTYPE){
            meshObject = "lineSphere";

            btScalar scaleAmount = ((btSphereShape*)shape)->getRadius();
            posScale = Ogre::Vector3(scaleAmount, scaleAmount, scaleAmount);
        }else{
            meshObject = "lineSphere";

            btScalar scaleAmount = ((btSphereShape*)shape)->getRadius();
            posScale = Ogre::Vector3(scaleAmount, scaleAmount, scaleAmount);
        }

        bodyNode->setScale(posScale);
        Ogre::Item *item = mSceneManager->createItem(meshObject, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::SCENE_DYNAMIC);
        assert(datablockId < NUM_CATEGORIES);
        item->setDatablock(mCategoryDatablocks[datablockId]);
        bodyNode->attachObject((Ogre::MovableObject*)item);

        bodyNode->setVisible(mVisibleOverride);

        return bodyNode;
    }

    void MeshVisualiser::insertNavMesh(dtNavMesh* mesh){
        assert(mAttachedNavMeshes.find(mesh) == mAttachedNavMeshes.end());
        Ogre::MeshPtr createdMesh = mNavMeshDebugDraw->produceMeshForNavMesh(mesh);

        Ogre::SceneNode* meshNode = mNavMeshObjectNode->createChildSceneNode();
        Ogre::Item *item = mSceneManager->createItem(createdMesh, Ogre::SCENE_DYNAMIC);
        meshNode->attachObject((Ogre::MovableObject*)item);

        item->setDatablock(mNavMeshDatablock);

        mAttachedNavMeshes[mesh] = meshNode;
    }

    void MeshVisualiser::removeNavMesh(dtNavMesh* mesh){
        auto it = mAttachedNavMeshes.find(mesh);
        assert(it != mAttachedNavMeshes.end());

        Ogre::SceneNode* node = (*it).second;
        assert(node);

        OgreNodeHelper::destroyNodeAndChildren(node);
        mAttachedNavMeshes.erase(it);
    }

    void MeshVisualiser::insertCollisionObjectChunk(const PhysicsTypes::CollisionChunkEntry& chunk){
        assert(chunk.first && chunk.second);
        assert(mAttachedCollisionObjectChunks.find(chunk) == mAttachedCollisionObjectChunks.end());

        Ogre::SceneNode* chunkNode = mCollisionObjectsChunkNode->createChildSceneNode();

        for(const btCollisionObject* b : *(chunk.second) ){
            uint8 worldId = CollisionWorldUtils::_readPackedIntWorldId(b->getUserIndex());
            _createSceneNode(chunkNode, b, worldId + 1);
        }

        mAttachedCollisionObjectChunks[chunk] = chunkNode;

        SlotPosition pos(chunk.slotX, chunk.slotY);
        chunkNode->_setDerivedPosition(pos.toOgre());
    }

    void MeshVisualiser::insertPhysicsChunk(const PhysicsTypes::PhysicsChunkEntry& chunk){
        assert(chunk.first && chunk.second);
        const PhysicsChunkContainer cont{chunk.first, chunk.second};
        assert(mAttachedPhysicsChunks.find(cont) == mAttachedPhysicsChunks.end());

        Ogre::SceneNode* chunkNode = mPhysicsChunkNode->createChildSceneNode();

        for(const btRigidBody* b : *(chunk.second) ){
            _createSceneNode(chunkNode, b, 0);
        }
        mAttachedPhysicsChunks[cont] = chunkNode;

        SlotPosition pos(chunk.slotX, chunk.slotY);
        chunkNode->_setDerivedPosition(pos.toOgre());
    }

    void MeshVisualiser::destroyPhysicsChunk(const PhysicsTypes::PhysicsChunkEntry& chunk){
        const PhysicsChunkContainer cont{chunk.first, chunk.second};
        auto it = mAttachedPhysicsChunks.find(cont);
        assert(it != mAttachedPhysicsChunks.end());

        Ogre::SceneNode* node = (*it).second;
        assert(node);

        OgreNodeHelper::destroyNodeAndChildren(node);
        mAttachedPhysicsChunks.erase(it);
    }

    void MeshVisualiser::setCollisionObjectPosition(const Ogre::Vector3& pos, const btCollisionObject* obj){
        auto it = mAttachedCollisionObjects.find(obj);
        if(it == mAttachedCollisionObjects.end()) return;

        (*it).second->setPosition(pos);
    }

    void _repositionMeshes(Ogre::SceneNode* targetNode, const Ogre::Vector3& offset){
        auto it = targetNode->getChildIterator();
        while(it.current() != it.end()){
            Ogre::SceneNode *node = (Ogre::SceneNode*)it.getNext();
            if(node->numChildren() == 0){
                node->setPosition(node->getPosition() - offset);
            }else{
                _repositionMeshes(node, offset);
            }

        }
    }

    void MeshVisualiser::_repositionMeshesOriginShift(const Ogre::Vector3& offset){
        _repositionMeshes(mParentNode, offset);
    }

    bool MeshVisualiser::worldEventReceiver(const Event &e){
        const WorldEvent& event = (WorldEvent&)e;
        if(event.eventId() == EventId::WorldOriginChange){
            const WorldEventOriginChange& originEvent = (WorldEventOriginChange&)event;

            _repositionMeshesOriginShift(originEvent.worldOffset);
        }
        return true;
    }
}

#endif
