#pragma once

#include "PhysicsWorld.h"
#include "Threading/Thread/Physics/DynamicsWorldThreadLogic.h"
#include "System/EnginePrerequisites.h"

#include "Entity/eId.h"

#include "System/Util/DataPacker.h"

#include <map>
#include <unordered_map>

namespace Ogre{
    class SceneNode;
};

namespace AV{
    class DynamicsWorldThreadLogic;
    class PhysicsBodyConstructor;
    
    /**
    Main thread exposure logic for the dynamics world.
    */
    class DynamicsWorld : public PhysicsWorld{
        friend PhysicsBodyConstructor;
    public:
        DynamicsWorld();
        ~DynamicsWorld();

        enum class BodyAttachObjectType{
            OBJECT_TYPE_NONE,
            OBJECT_TYPE_ENTITY,
            OBJECT_TYPE_MESH
        };

        struct EntityTransformData{
            eId entity;
            btVector3 pos;
            btQuaternion orientation;
        };

        struct MeshTransformData{
            Ogre::SceneNode* meshNode;
            btVector3 pos;
            btQuaternion orientation;
        };

        //Defines a thread safe copy for some queryable data. This will be mirrored on the main thread for each object in the world.
        struct DynamicsObjectWorldData{
            btVector3 position;
            btQuaternion orientation;
            btVector3 linearVelocity;
        };

        //Two bodies hitting each other hard enough to count. The order of the two means nothing.
        struct ImpactData{
            CollisionInternalId bodyA;
            CollisionInternalId bodyB;
            btVector3 pos;
            //The change in speed of the lighter body. Close to how fast they met.
            float strength;
        };

        void setDynamicsWorldThreadLogic(DynamicsWorldThreadLogic* dynLogic);

        void addBody(PhysicsTypes::RigidBodyPtr body);
        void removeBody(PhysicsTypes::RigidBodyPtr body);
        void setBodyPosition(PhysicsTypes::RigidBodyPtr body, btVector3 pos);
        void shiftOrigin(const btVector3& delta);
        DynamicsWorldThreadLogic::Statistics getStatistics() const;


        bool bodyInWorld(PhysicsTypes::RigidBodyPtr body) const;
        bool attachEntityToBody(PhysicsTypes::RigidBodyPtr body, eId e);
        void detatchEntityFromBody(PhysicsTypes::RigidBodyPtr body);
        bool attachMeshToBody(PhysicsTypes::RigidBodyPtr body, Ogre::SceneNode* meshNode);
        void detachMeshFromBody(PhysicsTypes::RigidBodyPtr body);
        BodyAttachObjectType getBodyBindType(PhysicsTypes::RigidBodyPtr body);

        void setBodyLinearFactor(PhysicsTypes::RigidBodyPtr body, btVector3 factor);
        void setBodyLinearVelocity(PhysicsTypes::RigidBodyPtr body, btVector3 velocity);

        const std::vector<EntityTransformData>& getEntityTransformData() { return mEntityTransformData; }
        /**
        Everything which hit something since the last update, replaced every update.
        */
        const std::vector<ImpactData>& getImpacts() const { return mImpacts; }
        /**
        Report contacts stronger than this, as a speed. Negative reports nothing, which is the default.
        */
        void setImpactThreshold(float strength);

        void update();

        btVector3 getBodyPosition(PhysicsTypes::RigidBodyPtr body);
        btVector3 getBodyLinearVelocity(PhysicsTypes::RigidBodyPtr body);
        btQuaternion getBodyOrientation(PhysicsTypes::RigidBodyPtr body);

        static CollisionInternalId getBodyInternalIdStatic(PhysicsTypes::RigidBodyPtr body);

    private:
        std::set<btRigidBody*> mBodiesInWorld;
        std::map<btRigidBody*, eId> mEntitiesInWorld;
        std::map<btRigidBody*, Ogre::SceneNode*> mMeshesInWorld;
        std::vector<EntityTransformData> mEntityTransformData;
        std::vector<MeshTransformData> mMeshTransformData;
        std::vector<ImpactData> mImpacts;
        std::set<btRigidBody*> mIgnoredBodies;
        //A thread safe copy of notable object properties.
        std::unordered_map<btRigidBody*, DynamicsObjectWorldData> mObjectWorldData;

        void _resetBufferEntries(btRigidBody* b);
        bool _attachToBody(btRigidBody* body, DynamicsWorld::BodyAttachObjectType type);
        void _detatchFromBody(btRigidBody* body);
        void _destroyBodyInternal(btRigidBody* bdy);

        /**
        Remove a body from the destruction world.
        This should only be called by shared pointers during their destruction.
        */
        static void _removeBody(btRigidBody* bdy);
        static DataPacker<PhysicsTypes::RigidBodyEntry>* mBodyData;
        static DynamicsWorld* dynWorld;

        inline bool _shouldIgnoreBody(btRigidBody* bdy);
        inline bool _bodyInWorld(btRigidBody* bdy) const;
        inline void _deleteBodyPtr(btRigidBody* bdy);

        DynamicsWorldThreadLogic* mDynLogic;
        btVector3 mOrigin{0, 0, 0};
    };
}
