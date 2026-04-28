#pragma once

#ifdef ENABLE_ADMOB

#include "ScriptUtils.h"

namespace AV {
    /**
     Scripting namespace for AdMob monetisation functions.
     Available as _monetisation in Squirrel scripts.

     Unit IDs should be configured early in your startup script:
         _monetisation.setBannerAdUnitId("ca-app-pub-xxx/yyy");
         _monetisation.setInterstitialAdUnitId("ca-app-pub-xxx/zzz");

     Ad lifecycle events fire to any scripts subscribed via _event.subscribe():
         _event.subscribe(2002, function(eventId, data) { ... }); //interstitial loaded
         _event.subscribe(2003, function(eventId, data) { ... }); //interstitial closed

     Event ID constants are also exposed as _monetisation.AD_EVENT_* for convenience.
    */
    class MonetisationNamespace {
    public:
        MonetisationNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        static SQInteger setBannerAdUnitId(HSQUIRRELVM vm);
        static SQInteger showBannerAd(HSQUIRRELVM vm);
        static SQInteger hideBannerAd(HSQUIRRELVM vm);

        static SQInteger setInterstitialAdUnitId(HSQUIRRELVM vm);
        static SQInteger loadInterstitialAd(HSQUIRRELVM vm);
        static SQInteger showInterstitialAd(HSQUIRRELVM vm);
        static SQInteger isInterstitialAdReady(HSQUIRRELVM vm);
    };
}

#endif //ENABLE_ADMOB
