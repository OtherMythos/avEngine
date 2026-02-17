#pragma once

namespace AV{
    enum PAUSE_TYPES{
        PAUSE_TYPE_PHYSICS = 1u << 0,
        PAUSE_TYPE_PHYSICS_DYNAMICS = 1u << 1,
        PAUSE_TYPE_PHYSICS_COLLISION = 1u << 2,
        PAUSE_TYPE_PHYSICS_COLLISION0 = 1u << 3,
        PAUSE_TYPE_PHYSICS_COLLISION1 = 1u << 4,
        PAUSE_TYPE_PHYSICS_COLLISION2 = 1u << 5,
        PAUSE_TYPE_PHYSICS_COLLISION3 = 1u << 6,
        PAUSE_TYPE_NAV_MESH = 1u << 7,
        PAUSE_TYPE_LIFETIME_COMPONENT = 1u << 8,
        PAUSE_TYPE_ANIMATIONS = 1u << 9,
        PAUSE_TYPE_TIMERS = 1u << 10,
        PAUSE_TYPE_ENTITY_UPDATE = 1u << 11,
        PAUSE_TYPE_PARTICLES = 1u << 12,
        PAUSE_TYPE_SPECIFIC_ANIMATIONS = 1u << 13,
    };

    /**SQNamespace
    @name _state
    */

    /**SQConstant
    @name _PAUSE_PHYSICS
    @desc Pause flag to pause all physics simulations, including dynamics and every collision world.
    */
    /**SQConstant
    @name _PAUSE_PHYSICS_DYNAMICS
    @desc Pause flag to pause all specifically dynamics.
    */
    /**SQConstant
    @name _PAUSE_PHYSICS_COLLISION
    @desc Pause flag to pause all specifically collision physics. This pauses all collision worlds.
    */
    /**SQConstant
    @name _PAUSE_PHYSICS_COLLISION0
    @desc Pause just collision world 0.
    */
    /**SQConstant
    @name _PAUSE_PHYSICS_COLLISION1
    @desc Pause just collision world 1.
    */
    /**SQConstant
    @name _PAUSE_PHYSICS_COLLISION2
    @desc Pause just collision world 2.
    */
    /**SQConstant
    @name _PAUSE_PHYSICS_COLLISION3
    @desc Pause just collision world 3.
    */
    /**SQConstant
    @name _PAUSE_NAV_MESH
    @desc Pause nav mesh calculations and processing.
    */
    /**SQConstant
    @name _PAUSE_LIFETIME_COMPONENT
    @desc Pause the countdown for entity lifetime components.
    */
    /**SQConstant
    @name _PAUSE_ANIMATIONS
    @desc Pause all animations currently run by the animation system.
    */
    /**SQConstant
    @name _PAUSE_TIMERS
    @desc Pause all timers.
    */
    /**SQConstant
    @name _PAUSE_ENTITY_UPDATE
    @desc Stop calling the entity update function for entities with a script attached.
    */
    /**SQConstant
    @name _PAUSE_PARTICLES
    @desc Stop particle animations.
    */
    /**SQConstant
    @name _PAUSE_SPECIFIC_ANIMATIONS
    @desc Pause specific animations based on individual animation pause masks.
    */
    static const char* const PauseTypeStrs[] = {
        "_PAUSE_PHYSICS",
        "_PAUSE_PHYSICS_DYNAMICS",
        "_PAUSE_PHYSICS_COLLISION",
        "_PAUSE_PHYSICS_COLLISION0",
        "_PAUSE_PHYSICS_COLLISION1",
        "_PAUSE_PHYSICS_COLLISION2",
        "_PAUSE_PHYSICS_COLLISION3",
        "_PAUSE_NAV_MESH",
        "_PAUSE_LIFETIME_COMPONENT",
        "_PAUSE_ANIMATIONS",
        "_PAUSE_TIMERS",
        "_PAUSE_ENTITY_UPDATE",
        "_PAUSE_PARTICLES",
        "_PAUSE_SPECIFIC_ANIMATIONS",
    };

    typedef unsigned int PauseMask;
}
