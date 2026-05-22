#pragma once

#include "Event.h"

namespace AV{
    class AdvertisingEvent : public Event{
    public:
        AV_EVENT_TYPE(EventType::System)
    };

    class AdvertisingEventBannerLoaded : public AdvertisingEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::AdvertisingBannerLoaded)
    };

    class AdvertisingEventBannerFailed : public AdvertisingEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::AdvertisingBannerFailed)
    };

    class AdvertisingEventInterstitialLoaded : public AdvertisingEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::AdvertisingInterstitialLoaded)
    };

    class AdvertisingEventInterstitialClosed : public AdvertisingEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::AdvertisingInterstitialClosed)
    };

    class AdvertisingEventInterstitialFailed : public AdvertisingEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::AdvertisingInterstitialFailed)
    };

    class AdvertisingEventRewardedLoaded : public AdvertisingEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::AdvertisingRewardedLoaded)
    };

    class AdvertisingEventRewardedClosed : public AdvertisingEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::AdvertisingRewardedClosed)
    };

    class AdvertisingEventRewardedFailed : public AdvertisingEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::AdvertisingRewardedFailed)
    };

    class AdvertisingEventRewardEarned : public AdvertisingEvent{
    public:
        AV_EVENT_TYPE(EventType::System)
        AV_EVENT_ID(EventId::AdvertisingRewardEarned)
    };
}
