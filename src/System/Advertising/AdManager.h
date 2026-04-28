#pragma once

#ifdef ENABLE_ADMOB

#include <string>
#include "Event/Events/Event.h"

namespace AV {
    /**
     Abstract interface for the ad manager. Platform-specific implementations
     inherit from this class. The null implementation (AdManagerNull) is used
     on platforms where AdMob is not yet implemented.

     Ad events are proper engine EventIds defined in Event.h:
     - AdvertisingBannerLoaded
     - AdvertisingBannerFailed
     - AdvertisingInterstitialLoaded
     - AdvertisingInterstitialClosed
     - AdvertisingInterstitialFailed

     Implementations dispatch these events via EventDispatcher::transmitEvent().
     Scripts subscribe using: _event.subscribe(_EVENT_ADVERTISING_*, callback, this)
    */
    class AdManager {
    public:
        virtual ~AdManager() = default;

        virtual void initialise() = 0;

        virtual void setBannerAdUnitId(const std::string& unitId) = 0;
        virtual void showBannerAd() = 0;
        virtual void hideBannerAd() = 0;

        virtual void setInterstitialAdUnitId(const std::string& unitId) = 0;
        virtual void loadInterstitialAd() = 0;
        virtual void showInterstitialAd() = 0;
        virtual bool isInterstitialAdReady() const = 0;

        /**
         Set/get the singleton instance. Called from Base during initialisation.
        */
        static void setInstance(AdManager* mgr);
        static AdManager* getInstance();

    private:
        static AdManager* sInstance;
    };
}

#endif //ENABLE_ADMOB
