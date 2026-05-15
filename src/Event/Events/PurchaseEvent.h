#pragma once

#include "Event.h"

#include <string>

namespace AV {
    class PurchaseEvent : public Event {
    public:
        AV_EVENT_TYPE(EventType::System)
    };

    class PurchaseEventProductPurchased : public PurchaseEvent {
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::PurchaseProductPurchased)
        std::string productId;
    };

    class PurchaseEventProductFailed : public PurchaseEvent {
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::PurchaseProductFailed)
        std::string productId;
    };

    class PurchaseEventRestoreCompleted : public PurchaseEvent {
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::PurchaseRestoreCompleted)
    };

    class PurchaseEventRestoreFailed : public PurchaseEvent {
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::PurchaseRestoreFailed)
    };

    class PurchaseEventProductInfo : public PurchaseEvent {
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::PurchaseProductInfo)
        std::string productId;
        std::string price;
        std::string title;
        std::string description;
    };

    class PurchaseEventProductInfoFailed : public PurchaseEvent {
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::PurchaseProductInfoFailed)
        std::string productId;
    };
}
