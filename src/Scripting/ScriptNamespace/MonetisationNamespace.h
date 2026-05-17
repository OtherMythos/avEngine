#pragma once

#ifdef ENABLE_MONETISATION

#include "ScriptUtils.h"

namespace AV {
    /**
     Scripting namespace for monetisation functions (AdMob and/or in-app purchases).
     Available as _monetisation in Squirrel scripts.

     AdMob (requires ENABLE_ADMOB):
         _monetisation.setBannerAdUnitId("ca-app-pub-xxx/yyy");
         _monetisation.setInterstitialAdUnitId("ca-app-pub-xxx/zzz");

     In-app purchases (requires ENABLE_MICROTRANSACTIONS):
         _monetisation.purchaseProduct("com.example.product");
         _monetisation.restorePurchases();
         _monetisation.queryProductInfo("com.example.product");

     Ad lifecycle events fire to any scripts subscribed via _event.subscribe().
    */
    class MonetisationNamespace {
    public:
        MonetisationNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
#ifdef ENABLE_ADMOB
        static SQInteger setBannerAdUnitId(HSQUIRRELVM vm);
        static SQInteger showBannerAd(HSQUIRRELVM vm);
        static SQInteger hideBannerAd(HSQUIRRELVM vm);
        static SQInteger getBannerAdBounds(HSQUIRRELVM vm);

        static SQInteger setInterstitialAdUnitId(HSQUIRRELVM vm);
        static SQInteger loadInterstitialAd(HSQUIRRELVM vm);
        static SQInteger showInterstitialAd(HSQUIRRELVM vm);
        static SQInteger isInterstitialAdReady(HSQUIRRELVM vm);

        static SQInteger setPersonalisedAds(HSQUIRRELVM vm);
        static SQInteger isPersonalisedAds(HSQUIRRELVM vm);
#endif

#ifdef ENABLE_MICROTRANSACTIONS
        static SQInteger purchaseProduct(HSQUIRRELVM vm);
        static SQInteger restorePurchases(HSQUIRRELVM vm);
        static SQInteger queryProductInfo(HSQUIRRELVM vm);
        static SQInteger canMakePayments(HSQUIRRELVM vm);
#endif
    };
}

#endif //ENABLE_MONETISATION
