#pragma once

#include <functional>
#include <vector>
#include <map>

#include "Events/Event.h"

namespace AV{

    #define AV_BIND(x) std::pair<std::function<bool(const Event&)>, void*>(std::bind(&x, this, std::placeholders::_1), this)
    #define AV_BIND_STATIC(x) std::function<bool(const Event&)>(x)

    /**
    Manages sending events to the interested parties.
    This includes subscription and transmission of events.
    */
    class EventDispatcher{
    public:
        typedef std::function<bool(const Event&)> FunctionType;
        typedef std::pair<FunctionType, void*> EventFunction;

        /**
        Subscribe the provided function to the provided event type.

        @param type
        The type of event the function wants to subscribe to.
        @param function
        A binding to the function to subscribe. This should use the AV_BIND macro.

        @remarks
        This function will return false and the function won't be bound if that instance already contains a binding of that event type to another function.

        @return
        Whether or not the subscription was successful or not.
        */
        static bool subscribe(EventType type, EventFunction function);
        static bool unsubscribe(EventType type, void* ptr);

        /**
        Subscribe a static function to the specified event type.
        */
        static bool subscribeStatic(EventType type, FunctionType function);
        static bool unsubscribeStatic(EventType type, FunctionType function);

        /**
        Transmit an event of a specific type to the subscribed members of that event type.

        @param type
        The event type to transmit.

        @param e
        The event to transmit.

        @remarks
        This function will check that the event being sent is of the correct type. If it is not the function will return false.

        @return
        True or false depending on whether the event was transmitted correctly.
        */
        static bool transmitEvent(EventType type, const Event &e);

    private:
        static bool _entryMapContains(EventType type, void* entryClass);
        static bool _entryMapContains(EventType type, const std::type_info& functionId);

        //A map of vectors for each event type. These vectors contain a list of functions to call for that event type.
        static std::map< EventType, std::vector<EventFunction> > entryMap;
        static std::map< EventType, std::vector<FunctionType> > mStaticEntryMap;
    };

}
