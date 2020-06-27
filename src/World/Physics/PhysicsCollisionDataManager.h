#pragma once

#include "World/Physics/Worlds/CollisionWorldUtils.h"
#include "Scripting/ScriptDataPacker.h"

#include <memory>

namespace AV{
    class CallbackScript;

    /**
    A class to manage the data associated with collision objects.
    This includes storage, as well as things like executing collision scripts, or playing sounds.
    */
    class PhysicsCollisionDataManager{
    public:
        PhysicsCollisionDataManager() = delete;
        ~PhysicsCollisionDataManager() = delete;

        static void shutdown();

        static void* createCollisionSenderScriptFromData(const std::string& scriptPath, const std::string& funcName, int id);

        static void processCollision(const btCollisionObject* sender, const btCollisionObject* receiver, CollisionObjectEventMask::CollisionObjectEventMask eventMask);

    private:
        struct CollisionSenderScriptEntry{
            std::shared_ptr<CallbackScript> scriptPtr;
            int closureId;
            int userIndex;
        };

        static ScriptDataPacker<CollisionSenderScriptEntry> mSenderScriptObjects;
    };
}
