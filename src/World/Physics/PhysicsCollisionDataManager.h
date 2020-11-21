#pragma once

#include "World/Physics/Worlds/CollisionWorldUtils.h"
#include "System/Util/DataPacker.h"
#include "Scripting/ScriptVM.h"
#include "System/EnginePrerequisites.h"

#include <memory>
#include <string>

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

        static void startup();
        static void shutdown();

        static void* createCollisionSenderScriptFromData(std::shared_ptr<CallbackScript> script, const std::string& funcName, int id);
        static void* createCollisionSenderScriptFromData(const std::string& scriptPath, const std::string& funcName, int id);
        static void* createCollisionSenderClosureFromData(SQObject closure, uint8 closureParams, int id);

        /**
        Process a collision between a sender and receiver, fulfilling any requirements for the collision such as a script call.
        Call this function when a collision has occured.
        */
        static void processCollision(const btCollisionObject* sender, const btCollisionObject* receiver, CollisionObjectEventMask::CollisionObjectEventMask eventMask);

        /**
        Set an override squirrel function to be called when a collision occurs.
        If set this function is always called rather than any set callback functions.
        */
        static void setCollisionCallbackOverride(SQObject closure);

        struct CollisionSenderUserData{
            int userIndex;
        };

        static const CollisionSenderUserData* _getCollisionDataOfObject(const btCollisionObject* obj);

    private:

        struct CollisionSenderScriptEntry{
            std::shared_ptr<CallbackScript> scriptPtr;
            int closureId;
            CollisionSenderUserData userData;
        };

        struct CollisionSenderClosureEntry{
            SQObject closure;
            uint8 numParams;
            CollisionSenderUserData userData;
        };

        static SQObject overrideFunction;

        static void _processCollisionScript(void* scriptEntry, CollisionObjectEventMask::CollisionObjectEventMask eventMask, int internalId);
        static void _processCollisionClosure(void* scriptEntry, CollisionObjectEventMask::CollisionObjectEventMask eventMask, int internalId);

        static bool _determinePopulateFunction(uint8 numParams, const CollisionSenderUserData& data, CollisionObjectEventMask::CollisionObjectEventMask eventMask, PopulateFunction* outFunc, int internalId);

        static DataPacker<CollisionSenderScriptEntry> mSenderScriptObjects;
        static DataPacker<CollisionSenderClosureEntry> mSenderClosureObjects;
    };
}
