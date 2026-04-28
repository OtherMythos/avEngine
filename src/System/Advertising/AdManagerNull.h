#pragma once

#include "AdManager.h"

#ifdef ENABLE_ADMOB

namespace AV {
    /**
     No-op AdManager used on platforms where AdMob is not yet implemented,
     or as a safe fallback when no platform implementation is available.
    */
    class AdManagerNull : public AdManager {
    public:
        void initialise() override {}

        void setBannerAdUnitId(const std::string& unitId) override {}
        void showBannerAd() override {}
        void hideBannerAd() override {}

        void setInterstitialAdUnitId(const std::string& unitId) override {}
        void loadInterstitialAd() override {}
        void showInterstitialAd() override {}
        bool isInterstitialAdReady() const override { return false; }
    };
}

#endif //ENABLE_ADMOB
