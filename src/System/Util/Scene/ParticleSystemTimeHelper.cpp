#include "ParticleSystemTimeHelper.h"

#include "OgreParticleSystem.h"

#include "OgreController.h"
#include "OgreControllerManager.h"
#include "Scripting/ScriptNamespace/StateNamespace.h"

namespace AV{

    /**
    Update particle systems based on whether they are globally paused.
    */
    class ParticleSystemUpdateValueAVEngine : public Ogre::ControllerValue<Ogre::Real>
    {
    protected:
        Ogre::ParticleSystem* mTarget;
    public:
        ParticleSystemUpdateValueAVEngine(Ogre::ParticleSystem* target) : mTarget(target) {}

        Ogre::Real getValue(void) const { return 0; } // N/A

        static bool update;
        void setValue(Ogre::Real value) { mTarget->_update(update ? value : 0); }

    };
    bool ParticleSystemUpdateValueAVEngine::update = true;

    /**
    Wrapper around particle systems so I can change the time controller type.
    */
    class avEngineParticleSystem : public Ogre::ParticleSystem{
    public:
        avEngineParticleSystem(Ogre::IdType id, Ogre::ObjectMemoryManager *objectMemoryManager, Ogre::SceneManager *manager,
                               const Ogre::String& resourceGroupName, ParticleSystemUpdateValueAVEngine* update) :
        Ogre::ParticleSystem(id, objectMemoryManager, manager, resourceGroupName) { }

        void setTimeController(Ogre::Controller<Ogre::Real>* c){
            //There should be no circumstance where I override a pre-existing controller.
            assert(!mTimeController);
            mTimeController = c;
        }
    };

    void ParticleSystemTimeHelper::notifyParticleSystemAttachment(Ogre::ParticleSystem* ps){
        Ogre::ControllerManager &mgr = Ogre::ControllerManager::getSingleton();
        Ogre::ControllerValueRealPtr updValue;
        updValue.reset( OGRE_NEW ParticleSystemUpdateValueAVEngine( ps ) );

        avEngineParticleSystem* sys = static_cast<avEngineParticleSystem*>(ps);
        sys->setTimeController(mgr.createFrameTimePassthroughController( updValue ));
    }

    void ParticleSystemTimeHelper::setUpdateParticles(bool update){
        ParticleSystemUpdateValueAVEngine::update = update;
    }
}
