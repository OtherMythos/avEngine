#include "PhysicsSenderClass.h"

#include "Scripting/ScriptObjectTypeTags.h"

namespace AV{

    ScriptDataPacker<PhysicsTypes::CollisionObjectPtr> PhysicsSenderClass::mObjectData;
    SQObject PhysicsSenderClass::senderClassObject;
    SQObject PhysicsSenderClass::receiverClassObject;

    void PhysicsSenderClass::setupClass(HSQUIRRELVM vm){

        { //Sender
            sq_newclass(vm, 0);

            //ScriptUtils::addFunction(vm, physicsShapeCompare, "_cmp");

            sq_settypetag(vm, -1, CollisionSenderTypeTag);

            sq_resetobject(&senderClassObject);
            sq_getstackobj(vm, -1, &senderClassObject);
            sq_addref(vm, &senderClassObject);
            sq_pop(vm, 1);
        }

        { //Receiver
            sq_newclass(vm, 0);

            sq_settypetag(vm, -1, CollisionReceiverTypeTag);

            sq_resetobject(&receiverClassObject);
            sq_getstackobj(vm, -1, &receiverClassObject);
            sq_addref(vm, &receiverClassObject);
            sq_pop(vm, 1);
        }
    }

    void PhysicsSenderClass::createInstanceFromPointer(HSQUIRRELVM vm, PhysicsTypes::CollisionObjectPtr object, bool receiver){
        sq_pushobject(vm, receiver ? receiverClassObject : senderClassObject);

        sq_createinstance(vm, -1);

        void* id = mObjectData.storeEntry(object);
        sq_setinstanceup(vm, -1, (SQUserPointer*)id);

        sq_setreleasehook(vm, -1, physicsObjectReleaseHook);

        //Remove the class object.
        sq_remove(vm, -2);
    }

    bool PhysicsSenderClass::getPointerFromInstance(HSQUIRRELVM vm, SQInteger index, PhysicsTypes::CollisionObjectPtr* outPtr, bool receiver){
        SQUserPointer p;
        //if(SQ_FAILED(sq_getinstanceup(vm, index, &p, receiver ? CollisionReceiverTypeTag : CollisionSenderTypeTag))) return false;
        //TODO Right now I'm not performing type tag checks. It seems with classes you have to specify the type you want, otherwise it fails.
        //Userdata is different. I need to re-think how I'm going to do these checks if it supports both.
        if(SQ_FAILED(sq_getinstanceup(vm, index, &p, 0))) return false;

        *outPtr = mObjectData.getEntry(p);

        return true;
    }

    SQInteger PhysicsSenderClass::physicsObjectReleaseHook(SQUserPointer p, SQInteger size){
        mObjectData.getEntry(p).reset();

        mObjectData.removeEntry(p);

        return 0;
    }
}
