#include "ScriptEventDataFactory.h"

#include "Event/Events/Event.h"
#include "Event/Events/WorldEvent.h"
#include "Event/Events/SystemEvent.h"
#include "Event/Events/PurchaseEvent.h"
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
            case EventId::SystemInputTouchBegan:{
                const SystemEventInputTouchBegan& event = static_cast<const SystemEventInputTouchBegan&>(e);
                sq_newtableex(vm, 2);

                PUSH(sq_pushinteger(vm, event.fingerId), "fingerId");
                PUSH(sq_pushinteger(vm, event.guiIntersected), "guiIntersect");
                POP_TABLE();
                break;
            }
            case EventId::SystemInputTouchEnded:
            case EventId::SystemInputTouchMotion:{
                const SystemEventInputTouch& event = static_cast<const SystemEventInputTouch&>(e);
                sq_pushinteger(vm, event.fingerId);
                POP_TABLE();
                break;
            }
            default:{
                //Return just a null object.
                break;
            }

            case EventId::PurchaseProductPurchased:{
                const PurchaseEventProductPurchased& event = static_cast<const PurchaseEventProductPurchased&>(e);
                sq_newtableex(vm, 1);
                PUSH(sq_pushstring(vm, _SC(event.productId.c_str()), -1), "productId");
                POP_TABLE();
                break;
            }
            case EventId::PurchaseProductFailed:{
                const PurchaseEventProductFailed& event = static_cast<const PurchaseEventProductFailed&>(e);
                sq_newtableex(vm, 1);
                PUSH(sq_pushstring(vm, _SC(event.productId.c_str()), -1), "productId");
                POP_TABLE();
                break;
            }
            case EventId::PurchaseRestoreCompleted:
            case EventId::PurchaseRestoreFailed:{
                //No additional payload for restore events.
                break;
            }
            case EventId::PurchaseProductInfo:{
                const PurchaseEventProductInfo& event = static_cast<const PurchaseEventProductInfo&>(e);
                sq_newtableex(vm, 4);
                PUSH(sq_pushstring(vm, _SC(event.productId.c_str()), -1), "productId");
                PUSH(sq_pushstring(vm, _SC(event.price.c_str()), -1), "price");
                PUSH(sq_pushstring(vm, _SC(event.title.c_str()), -1), "title");
                PUSH(sq_pushstring(vm, _SC(event.description.c_str()), -1), "description");
                POP_TABLE();
                break;
            }
            case EventId::PurchaseProductInfoFailed:{
                const PurchaseEventProductInfoFailed& event = static_cast<const PurchaseEventProductInfoFailed&>(e);
                sq_newtableex(vm, 1);
                PUSH(sq_pushstring(vm, _SC(event.productId.c_str()), -1), "productId");
                POP_TABLE();
                break;
            }
        }

        return obj;
    }
}
