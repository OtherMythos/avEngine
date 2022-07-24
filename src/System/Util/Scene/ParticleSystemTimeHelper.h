#pragma once

namespace Ogre{
    class ParticleSystem;
}

namespace AV{

    /**
    A helper script for Ogre particle systems.
    In order to provide proper pause functionality, we have to assign them a custom Ogre ControllerValue function when attached.
    This allows the engine fine grain control over the specific controllers which are paused.
    It is possible to pause the ControllerManager class to also pause particle systems, however this can pause other things, such as shaders.

    Ogre destroys the controller manager as the particle system is attached and re-attached,
    so this has to be called each time the particle system is attached to a node.
    */
    class ParticleSystemTimeHelper{
    public:
        ParticleSystemTimeHelper() = delete;
        ~ParticleSystemTimeHelper() = delete;

        static void notifyParticleSystemAttachment(Ogre::ParticleSystem* ps);
        static void setUpdateParticles(bool updateParticles);
    };
}
