#pragma once

#include "Event/Events/Event.h"
#include <squirrel.h>

#include <map>
#include <queue>

namespace AV{
    class Event;

    /**
    Manages script exposure to events.
    Scripts can subscribe to system events as well as user events.
    Closures and their contexts are stored in this manager.
    */
    class ScriptEventManager{
    public:
        ScriptEventManager();
        ~ScriptEventManager();

        void subscribeEvent();

        /**
        Subscribe to a system event.
        @note You must remember to unsubscribe the closure when done, otherwise it will leak memory.
        @param closure The closure which should be subscribed to this event.
        @param context The context of the event. This can also be OT_NULL, OT_INSTANCE or OT_TABLE. If OT_NULL, the parameter is effectively ignored.
        */
        void subscribeEvent(EventId event, SQObject closure, SQObject context);
        /**
        Subscribe to a user event.
        @param event The id can be any arbuitrary integer, and will be called when an event is transmitted with that id.
        */
        void subscribeEvent(int event, SQObject closure, SQObject context);
        /**
        Unsubscribe a user event
        */
        bool unsubscribeEvent(EventId event);
        bool unsubscribeEvent(int event, SQObject closure);

        /**
        Transmit a user event.
        @param event: The event id. This must be greater than 1000.
        @param data: Generic data to transmit with the event.
        */
        bool transmitEvent(int event, SQObject data);

        void processQueuedEvents();

    private:
        bool eventReceiver(const Event &e);

        typedef std::pair<SQObject, SQObject> SubscribeEventEntry;
        std::map<EventId, SubscribeEventEntry> mSubscribeMap;
        bool mSubscribedEventTypes[static_cast<size_t>(EventId::EVENT_ID_END)];

        std::multimap<int, SubscribeEventEntry> mUserSubscribeMap;

        struct QueuedEventEntry{
            EventId id;
            //The data for this event. This will either be a table or null.
            SQObject data;
        };
        std::queue<QueuedEventEntry> mQueuedEvents;

        void _queueEvent(const Event& e);
        void _callQueuedEvent(const QueuedEventEntry& e);
    };
}
