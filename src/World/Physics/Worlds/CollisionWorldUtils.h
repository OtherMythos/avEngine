#pragma once

namespace AV{
    namespace CollisionObjectType{
        //Types a collision object can be.
        enum CollisionObjectType : char{
            RECEIVER,
            SENDER_SCRIPT, //When sent a script will be executed.
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
    }

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
    }

    namespace CollisionObjectEventMask{
        enum CollisionObjectEventMask : char{
            LEAVE   = 1u << CollisionObjectEvent::LEAVE,
            ENTER   = 1u << CollisionObjectEvent::ENTER,
            INSIDE  = 1u << CollisionObjectEvent::INSIDE
        };
    }

    namespace CollisionWorldUtils{
        /**
        Produce an integer value packed with the provided values.
        */
        int producePackedInt(CollisionObjectType::CollisionObjectType type, char objectTypeMask, char eventMask){
            int outValue = 0;

            outValue |= type;

            outValue |= objectTypeMask << 8;
            outValue |= eventMask << 16;

            return outValue;
        }
    }
}
