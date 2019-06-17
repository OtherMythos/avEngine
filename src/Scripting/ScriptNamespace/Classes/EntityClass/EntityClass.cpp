#include "EntityClass.h"

#include "World/WorldSingleton.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"
#include "World/Entity/EntityManager.h"

#include "World/Slot/ChunkRadiusLoader.h"
#include "World/Entity/Logic/FundamentalLogic.h"

namespace AV{

    SQObject EntityClass::classObject;

    ScriptDataPacker<uint64_t> EntityClass::eIdData;

    SQInteger EntityClass::setEntityPosition(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            eId entityId = getEID(vm, -2);

            SlotPosition pos = SlotPositionClass::getSlotFromInstance(vm, -1);

            world->getEntityManager()->setEntityPosition(entityId, pos);
        }
        return 0;
    }

    SQInteger EntityClass::getEntityPosition(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            eId entityId = getEID(vm, -1);

            SlotPosition pos = FundamentalLogic::getPosition(entityId);

            //Push a slotPositionClass instance to the stack.
            SlotPositionClass::instanceFromSlotPosition(vm, pos);

            return 1;
        }
        return 0;
    }

    SQInteger EntityClass::checkValid(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            eId entityId = getEID(vm, -1);

            SQBool result = world->getEntityManager()->getEntityValid(entityId);
            sq_pushbool(vm, result);

            return 1;
        }
        return 0;
    }

    void EntityClass::invalidateEntityInstance(HSQUIRRELVM vm){
        SQUserPointer p = 0;
        sq_getinstanceup(vm, -1, &p, 0);

        eIdData.setEntry(p, 0);
    }

    SQInteger EntityClass::checkTrackable(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            eId entityId = getEID(vm, -1);

            SlotPosition pos = FundamentalLogic::getPosition(entityId);
            bool viableChunk = WorldSingleton::getWorld()->getChunkRadiusLoader()->chunkLoadedInCurrentMap(pos.chunkX(), pos.chunkY());

            sq_pushbool(vm, viableChunk);

            return 1;
        }
        return 0;
    }

    eId EntityClass::getEID(HSQUIRRELVM vm, int stackIndex){
        //TODO investigate the type tag
        SQUserPointer p;
        sq_getinstanceup(vm, stackIndex, &p, 0);

        eId e(eIdData.getEntry(p));
        return e;
    }

    SQInteger EntityClass::isTracked(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            eId entityId = getEID(vm, -1);

            SQBool tracked = FundamentalLogic::getTracked(entityId);

            sq_pushbool(vm, tracked);

            return 1;
        }
        return 0;
    }

    SQInteger EntityClass::moveEntity(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            SQFloat x, y, z;

            sq_getfloat(vm, -1, &z);
            sq_getfloat(vm, -2, &y);
            sq_getfloat(vm, -3, &x);

            eId entityId = getEID(vm, -4);

            SlotPosition pos = FundamentalLogic::getPosition(entityId);
            pos = pos + Ogre::Vector3(1, 0, 0);

            world->getEntityManager()->setEntityPosition(entityId, pos);
        }
        return 0;
    }

    SQInteger EntityClass::_entityCompare(HSQUIRRELVM vm){
        SQUserPointer pf, ps;
        sq_getinstanceup(vm, -1, &pf, 0);
        sq_getinstanceup(vm, -2, &ps, 0);

        uint64_t first = eIdData.getEntry((void*)pf);
        uint64_t second = eIdData.getEntry((void*)ps);

        if(first == second){
            sq_pushinteger(vm, 0);
        }else{
            sq_pushbool(vm, false);
        }
        return 1;
    }

    void EntityClass::_entityClassFromEID(HSQUIRRELVM vm, eId entity){
        sq_pushobject(vm, classObject);

        sq_createinstance(vm, -1);

        void* id = eIdData.storeEntry(entity.id());
        sq_setinstanceup(vm, -1, (SQUserPointer*)id);

        sq_setreleasehook(vm, -1, EIDReleaseHook);
    }

    SQObject EntityClass::_objFromEID(HSQUIRRELVM vm, eId entity){
        _entityClassFromEID(vm, entity);

        SQObject obj;
        sq_getstackobj(vm, -1, &obj);
        sq_addref(vm, &obj);
        sq_pop(vm, 1);

        return obj;
    }

    SQInteger EntityClass::EIDReleaseHook(SQUserPointer p, SQInteger size){
        eIdData.removeEntry((void*)p);

        return 0;
    }

    void EntityClass::setupClass(HSQUIRRELVM vm){
        sq_newclass(vm, 0);

        sq_pushstring(vm, _SC("setPosition"), -1);
        sq_newclosure(vm, setEntityPosition, 0);
        sq_setparamscheck(vm,2,_SC(".x"));
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("getPosition"), -1);
        sq_newclosure(vm, getEntityPosition, 0);
        sq_setparamscheck(vm,0,_SC("."));
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("move"), -1);
        sq_newclosure(vm, moveEntity, 0);
        sq_setparamscheck(vm,4,_SC(".nnn"));
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("_cmp"), -1);
        sq_newclosure(vm, _entityCompare, 0);
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("valid"), -1);
        sq_newclosure(vm, checkValid, 0);
        sq_setparamscheck(vm,1,_SC("."));
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("trackable"), -1);
        sq_newclosure(vm, checkTrackable, 0);
        sq_setparamscheck(vm,1,_SC("."));
        sq_newslot(vm, -3, false);

        sq_pushstring(vm, _SC("tracked"), -1);
        sq_newclosure(vm, isTracked, 0);
        sq_setparamscheck(vm,1,_SC("."));
        sq_newslot(vm, -3, false);

        sq_resetobject(&classObject);
        sq_getstackobj(vm, -1, &classObject);
        sq_addref(vm, &classObject);
        sq_pop(vm, 1);
    }
}
