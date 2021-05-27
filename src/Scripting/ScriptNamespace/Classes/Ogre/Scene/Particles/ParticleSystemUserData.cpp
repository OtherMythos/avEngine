#include "ParticleSystemUserData.h"

#include "OgreParticleSystem.h"
#include "Scripting/ScriptObjectTypeTags.h"
#include "OgreSceneManager.h"
#include "OgreRoot.h"

namespace AV{

    SQObject ParticleSystemUserData::particleSystemDelegateTable;

    SQInteger ParticleSystemUserData::sqPhysicsShapeReleaseHook(SQUserPointer p, SQInteger size){
        //Destroy from the scene manager.
        Ogre::SceneManager* sceneManager = Ogre::Root::getSingletonPtr()->getSceneManager("Scene Manager");
        sceneManager->destroyParticleSystem( (Ogre::ParticleSystem*)p );

        return 0;
    }

    void ParticleSystemUserData::createUserDataFromPointer(HSQUIRRELVM vm, Ogre::ParticleSystem* system){
        /*Ogre::ParticleSystem** pointer = (Ogre::ParticleSystem**)sq_newuserdata(vm, sizeof(Ogre::ParticleSystem*));
        *pointer = system;

        sq_pushobject(vm, particleSystemDelegateTable);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, ParticleSystemTypeTag);*/
    }

    UserDataGetResult ParticleSystemUserData::getPointerFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::ParticleSystem** outObject){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != ParticleSystemTypeTag){
            *outObject = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        Ogre::ParticleSystem** p = static_cast<Ogre::ParticleSystem**>(pointer);
        *outObject = *p;

        return USER_DATA_GET_SUCCESS;
    }

    void ParticleSystemUserData::setupDelegateTable(HSQUIRRELVM vm, SQObject* obj){
        //sq_newtableex(vm, 4);
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, fastForward, "fastForward", 2, ".n");

        sq_resetobject(obj);
        sq_getstackobj(vm, -1, obj);
        sq_addref(vm, obj);
        sq_pop(vm, 1);

        particleSystemDelegateTable = *obj;
    }

    SQInteger ParticleSystemUserData::fastForward(HSQUIRRELVM vm){
        SQFloat value;
        sq_getfloat(vm, 2, &value);

        Ogre::ParticleSystem *ps;
        SCRIPT_ASSERT_RESULT(getPointerFromUserData(vm, 1, &ps));

        ps->fastForward(value);

        return 0;
    }
}
