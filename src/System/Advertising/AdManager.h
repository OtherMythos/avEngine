#pragma once

#ifdef ENABLE_ADMOB

#include <string>

namespace AV {
    class ScriptEventManager;

    /**
     Abstract interface for the ad manager. Platform-specific implementations
     inherit from this class. The null implementation (AdManagerNull) is used
     on platforms where AdMob is not yet implemented.

     Ad event IDs are user-event IDs that scripts can subscribe to via
     _event.subscribe(). They must be > 1000 per the ScriptEventManager contract.
    */
    class AdManager {
    public:
        static constexpr int AD_EVENT_BANNER_LOADED = 2000;
        static constexpr int AD_EVENT_BANNER_FAILED = 2001;
        static constexpr int AD_EVENT_INTERSTITIAL_LOADED = 2002;
        static constexpr int AD_EVENT_INTERSTITIAL_CLOSED = 2003;
        static constexpr int AD_EVENT_INTERSTITIAL_FAILED = 2004;

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
         Fire an ad event to any subscribed Squirrel scripts.
         Called by platform implementations when an ad lifecycle event occurs.
        */
        static void notifyAdEvent(int eventId);

        /**
         Set/get the singleton instance. Called from Base during initialisation.
        */
        static void setInstance(AdManager* mgr);
        static AdManager* getInstance();

        //Set by ScriptManager so notifyAdEvent can reach the script event system.
        static ScriptEventManager* mScriptEventManager;

    private:
        static AdManager* sInstance;
    };
}

#endif //ENABLE_ADMOB
