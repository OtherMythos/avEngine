#pragma once

#include "Event/Events/Event.h"
#include <squirrel.h>

#include <map>
#include <queue>

namespace AV{
    class Event;

    class ScriptEventManager{
    public:
        ScriptEventManager();
        ~ScriptEventManager();

        void subscribeEvent();

        void subscribeEvent(EventId event, SQObject closure, SQObject context);
        void subscribeEvent(int event, SQObject closure, SQObject context);
        void unsubscribeEvent(EventId event);
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
