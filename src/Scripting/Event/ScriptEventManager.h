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
        void unsubscribeEvent(EventId event);

        void processQueuedEvents();

    private:
        bool eventReceiver(const Event &e);

        typedef std::pair<SQObject, SQObject> SubscribeEventEntry;
        std::map<EventId, SubscribeEventEntry> mSubscribeMap;
        bool mSubscribedEventTypes[static_cast<size_t>(EventId::EVENT_ID_END)];

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
