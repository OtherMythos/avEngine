#pragma once

#include "World/Physics/Worlds/CollisionWorldUtils.h"
#include "Scripting/ScriptDataPacker.h"
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

        static void shutdown();

        static void* createCollisionSenderScriptFromData(std::shared_ptr<CallbackScript> script, const std::string& funcName, int id);
        static void* createCollisionSenderScriptFromData(const std::string& scriptPath, const std::string& funcName, int id);
        static void* createCollisionSenderClosureFromData(SQObject closure, uint8 closureParams, int id);

        static void processCollision(const btCollisionObject* sender, const btCollisionObject* receiver, CollisionObjectEventMask::CollisionObjectEventMask eventMask);

    private:
        struct CollisionSenderUserData{
            int userIndex;
        };

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

        static void _processCollisionScript(void* scriptEntry, CollisionObjectEventMask::CollisionObjectEventMask eventMask);
        static void _processCollisionClosure(void* scriptEntry, CollisionObjectEventMask::CollisionObjectEventMask eventMask);

        static bool _determinePopulateFunction(uint8 numParams, const CollisionSenderUserData& data, CollisionObjectEventMask::CollisionObjectEventMask eventMask, PopulateFunction* outFunc);

        static ScriptDataPacker<CollisionSenderScriptEntry> mSenderScriptObjects;
        static ScriptDataPacker<CollisionSenderClosureEntry> mSenderClosureObjects;
    };
}
