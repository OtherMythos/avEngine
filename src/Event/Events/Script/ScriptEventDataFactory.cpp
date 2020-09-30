#include "ScriptEventDataFactory.h"

#include "Event/Events/Event.h"
#include "Event/Events/WorldEvent.h"
#include "Event/Events/SystemEvent.h"
#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"

#define PUSH(xx, vv) sq_pushstring(vm, vv, -1); xx; sq_newslot(vm,-3,SQFalse);
#define POP_TABLE() sq_getstackobj(vm, -1, &obj); sq_addref(vm, &obj); sq_pop(vm, 1);

namespace AV{
    SQObject ScriptEventDataFactory::produceDataForEvent(HSQUIRRELVM vm, const Event& e){
        SQObject obj;
        sq_resetobject(&obj);
        switch(e.eventId()){

            //World events
            case EventId::WorldMapChange:{
                const WorldEventMapChange& event = static_cast<const WorldEventMapChange&>(e);
                sq_newtableex(vm, 2);

                PUSH(sq_pushstring(vm, _SC(event.newMapName.c_str()), -1), "new");
                PUSH(sq_pushstring(vm, _SC(event.oldMapName.c_str()), -1), "old");
                POP_TABLE();
                break;
            }
            case EventId::WorldOriginChange:{
                const WorldEventOriginChange& event = static_cast<const WorldEventOriginChange&>(e);
                sq_newtableex(vm, 2);

                PUSH(SlotPositionClass::createNewInstance(vm, event.newPos), "new");
                PUSH(SlotPositionClass::createNewInstance(vm, event.oldPos), "old");
                POP_TABLE();
                break;
            }
            case EventId::WorldPlayerRadiusChange:{
                const WorldEventPlayerRadiusChange& event = static_cast<const WorldEventPlayerRadiusChange&>(e);
                sq_newtableex(vm, 2);

                PUSH(sq_pushinteger(vm, event.newRadius), "new");
                PUSH(sq_pushinteger(vm, event.oldRadius), "old");
                POP_TABLE();
                break;
            }

            //System events
            case EventId::SystemWindowResize:{
                const SystemEventWindowResize& event = static_cast<const SystemEventWindowResize&>(e);
                sq_newtableex(vm, 2);

                PUSH(sq_pushinteger(vm, event.width), "width");
                PUSH(sq_pushinteger(vm, event.height), "height");
                POP_TABLE();
                break;
            }
            default:{
                //Return just a null object.
                break;
            }
        }

        return obj;
    }
}
