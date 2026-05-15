#pragma once

#ifdef ENABLE_ADMOB
#ifdef TARGET_APPLE_IPHONE

#include "System/Advertising/AdManager.h"

#include <string>

//Forward declare the Objective-C wrapper to avoid importing ObjC headers here.
#ifdef __OBJC__
@class GADBannerView;
@class GADInterstitialAd;
@class iOSAdManagerDelegate;
#endif

namespace AV {
    /**
     iOS implementation of AdManager using the Google Mobile Ads SDK.
     The Objective-C bridge objects are owned by the delegate helper class
     to keep this header free of Objective-C types.
    */
    class iosAdManager : public AdManager {
    public:
        iosAdManager();
        ~iosAdManager();

        void initialise() override;

        void setBannerAdUnitId(const std::string& unitId) override;
        void showBannerAd() override;
        void hideBannerAd() override;
        BannerAdBounds getBannerAdBounds() const override;

        void setInterstitialAdUnitId(const std::string& unitId) override;
        void loadInterstitialAd() override;
        void showInterstitialAd() override;
        bool isInterstitialAdReady() const override;

    //Member variables accessed by the Objective-C delegate
    public:
        std::string mBannerUnitId;
        std::string mInterstitialUnitId;
        bool mInterstitialReady = false;
        bool mBannerVisible = false;

    private:
        //Opaque pointer to the Objective-C delegate/helper object.
        void* mDelegate = nullptr;

        friend class iOSAdDelegate;
    };
}

#endif //TARGET_APPLE_IPHONE
#endif //ENABLE_ADMOB
