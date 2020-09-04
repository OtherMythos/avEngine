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

        struct QueuedEventEntry{
            EventId id;
            int i;
        };
        std::queue<QueuedEventEntry> mQueuedEvents;

        void _queueEvent(const Event& e);
        void _callQueuedEvent(const QueuedEventEntry& e);
    };
}