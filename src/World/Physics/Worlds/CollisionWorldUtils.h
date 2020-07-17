#pragma once

#include "World/Physics/PhysicsTypes.h"
#include "System/EnginePrerequisites.h"
#include <cassert>

namespace AV{
    namespace CollisionObjectType{
        //Types a collision object can be.
        enum CollisionObjectType : char{
            RECEIVER,
            SENDER_SCRIPT, //When sent a script will be executed.
            SENDER_CLOSURE //Specifically for when a closure object should be called, rather than an entire script.
            /*
            The following are a plan for the future.
            When I come to write a sound system, this will be the sort of thing the collision world will be capable of.
            While squirrel will contain functions to play sound, this will be a more direct and efficient method of achieving this goal as having devoted types means it can happen entirely in c++.
            */
            //SENDER_SOUND_EMITTER, //Constantly emits a sound based on the player's proximity to the sender. This has an outer and inner emitter with set radius which allows sounds to be played.
            //SENDER_SOUND_PRODUCER, //Emits a sound once.
        };
    }

    namespace CollisionObjectTarget{
        /**
        Specifies a number of types which senders and receivers can use to refine their eventual collision rate.
        For instance, a receiver would specify what types it receives, and senders would specify what types they send to.
        This helps to reduce the amount of noise quite significantly.
        Some sane default types have been provided. Over time more of these will be filled in as I come up with more.
        */
        enum CollisionObjectTarget{
            PLAYER,
            ENEMY,
            OBJECT,
            USER_3,
            USER_4,
            USER_5,
            USER_6
        };
        static const uint8 numTargetNames = 7;
        static const char* targetNames[numTargetNames] = {
            "Player",
            "Enemy",
            "Object",
            "User3",
            "User4",
            "User5",
            "User6"
        };
    }

    //TODO this should be targetMask.
    namespace CollisionObjectTypeMask{
        enum CollisionObjectTypeMask : char{
            PLAYER  = 1u << CollisionObjectTarget::PLAYER,
            ENEMY   = 1u << CollisionObjectTarget::ENEMY,
            OBJECT  = 1u << CollisionObjectTarget::OBJECT,
            USER_3  = 1u << CollisionObjectTarget::USER_3,
            USER_4  = 1u << CollisionObjectTarget::USER_4,
            USER_5  = 1u << CollisionObjectTarget::USER_5,
            USER_6  = 1u << CollisionObjectTarget::USER_6,
        };
    }

    namespace CollisionObjectEvent{
        /**
        Specifies when events for this type should be sent.
        This is stored in the sender object.
        */
        enum CollisionObjectEvent{
            LEAVE,
            ENTER,
            INSIDE
        };
        static const uint8 numObjectEvents = 3;
        static const char* eventNames[numObjectEvents] = {
            "Leave",
            "Enter",
            "Inside"
        };
    }

    namespace CollisionObjectEventMask{
        enum CollisionObjectEventMask : char{
            LEAVE   = 1u << CollisionObjectEvent::LEAVE,
            ENTER   = 1u << CollisionObjectEvent::ENTER,
            INSIDE  = 1u << CollisionObjectEvent::INSIDE
        };
    }


    namespace CollisionWorldUtils{
        struct PackedIntContents{
            CollisionObjectType::CollisionObjectType type;
            char target;
            char eventType;
            uint8 worldId;
        };

        /**
        Produce an integer value packed with the provided values.
        */
        static CollisionPackedInt producePackedInt(CollisionObjectType::CollisionObjectType type, uint8 collisionWorldId, char objectTypeMask, char eventMask){
            CollisionPackedInt outValue = 0;

            outValue |= type;

            outValue |= objectTypeMask << 8;
            outValue |= eventMask << 16;
            outValue |= collisionWorldId << 24;

            return outValue;
        }

        static inline CollisionObjectType::CollisionObjectType _readPackedIntType(CollisionPackedInt packedInt){
            return static_cast<CollisionObjectType::CollisionObjectType>(packedInt & 0xFF);
        }

        static inline char _readPackedIntTarget(CollisionPackedInt packedInt){
            return static_cast<char>((packedInt >> 8) & 0xFF);
        }

        static inline char _readPackedIntEventType(CollisionPackedInt packedInt){
            return static_cast<char>((packedInt >> 16) & 0xFF);
        }

        static inline uint8 _readPackedIntWorldId(CollisionPackedInt packedInt){
            return static_cast<uint8>((packedInt >> 24) & 0xFF);
        }

        /**
        Read the contents of a packed integer value.

        @param outContents
        The output value.
        */
        static void readPackedInt(CollisionPackedInt packedInt, PackedIntContents* outContents){
            outContents->type = _readPackedIntType(packedInt);
            outContents->target = _readPackedIntTarget(packedInt);
            outContents->eventType = _readPackedIntEventType(packedInt);
            outContents->worldId = _readPackedIntWorldId(packedInt);
        }

        static inline bool shouldObjectsSendEvent(CollisionObjectEventMask::CollisionObjectEventMask eventType, CollisionPackedInt a, CollisionPackedInt b){
            CollisionObjectType::CollisionObjectType typeA = _readPackedIntType(a);
            CollisionObjectType::CollisionObjectType typeB = _readPackedIntType(b);

            //OPTIMISATION (maybe)
            //These two checks are more expensive than 1. If this code is part of the heavily run code I could consider reducing the enums to the two types again.
            //Previously I could just do if(typeA == typeB) return, because there were only two.
            if(typeA == CollisionObjectType::RECEIVER && typeB == CollisionObjectType::RECEIVER) return false;
            if(typeA > 0 && typeB > 0) return false;

            //One of them should be something other than a receiver, i.e a sender.
            assert(typeA != CollisionObjectType::RECEIVER || typeB != CollisionObjectType::RECEIVER);

            char senderEventType = typeA == CollisionObjectType::RECEIVER ? _readPackedIntEventType(b) : _readPackedIntEventType(a);
            //If the event does not match the requested one.
            if( (senderEventType & eventType) == 0) return false;

            char senderType = _readPackedIntTarget(a);
            char receiverType = _readPackedIntTarget(b);
            //Finally check the object types of the senders and receivers match.
            if( (senderType & receiverType) == 0) return false;

            //It passed all these checks, so...
            return true;
        }

        /**
        Produce packed info specifically for the thread. This will be attached to the userIndex2, and never read by the main thread.
        */
        static CollisionPackedInt produceThreadPackedInt(bool inWorld){
            //They don't really do any packing right now. However it's worth going through this infrustructure regardless.
            return inWorld ? 1 : 0;
        }

        static inline bool _readPackedIntInWorld(CollisionPackedInt target){
            return target > 0;
        }
    }
}
