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

#include "Physics/Worlds/CollisionWorldUtils.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"

#include "System/Util/OgreNodeHelper.h"
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
        : mVisibleOverride(true),
        mRenderQueue(0) {
        EventDispatcher::subscribe(EventType::DebuggerTools, AV_BIND(MeshVisualiser::debuggerToolsReceiver));

        for(int i = 0; i < MAX_COLLISION_WORLDS; i++)
            mCollisionWorldObjectNodes[i] = 0;
    }

    MeshVisualiser::~MeshVisualiser(){
        //This destruction happens during a complete shutdown, so it's not a problem to completely wipe the list.

        //Destroy parent nodes. Their children should already be destroyed.
        for(int i = 0; i < MAX_COLLISION_WORLDS; i++){
            if(!mCollisionWorldObjectNodes[i]) continue;
            OgreNodeHelper::recursiveDestroyNode(mCollisionWorldObjectNodes[i]);
            mSceneManager->destroySceneNode(mCollisionWorldObjectNodes[i]);
        }

        mAttachedCollisionObjects.clear();

        Ogre::Hlms* hlms = Ogre::Root::getSingletonPtr()->getHlmsManager()->getHlms(Ogre::HLMS_UNLIT);
        for(const char* d : mDatablockNames){
            hlms->destroyDatablock(d);
        }
        EventDispatcher::unsubscribe(EventType::DebuggerTools, this);
    }

    void MeshVisualiser::initialise(Ogre::SceneManager* sceneManager){
        mSceneManager = sceneManager;

        mParentNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
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

    }

    void MeshVisualiser::setMeshGroupVisible(MeshGroupType type, bool visible){
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
        item->setRenderQueueGroup(mRenderQueue);
        bodyNode->attachObject((Ogre::MovableObject*)item);

        bodyNode->setVisible(mVisibleOverride);

        return bodyNode;
    }

    void MeshVisualiser::setRenderQueueForMeshes(uint8 meshGroup){
        mRenderQueue = meshGroup;
    }

    void MeshVisualiser::setCollisionObjectPosition(const Ogre::Vector3& pos, const btCollisionObject* obj){
        auto it = mAttachedCollisionObjects.find(obj);
        if(it == mAttachedCollisionObjects.end()) return;

        (*it).second->setPosition(pos);
    }

}

#endif
