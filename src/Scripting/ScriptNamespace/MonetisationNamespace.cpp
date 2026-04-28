#ifdef ENABLE_ADMOB

#include "MonetisationNamespace.h"
#include "ScriptUtils.h"
#include "System/Advertising/AdManager.h"
#include "Event/Events/Event.h"

namespace AV {

    SQInteger MonetisationNamespace::setBannerAdUnitId(HSQUIRRELVM vm){
        const SQChar* unitId;
        sq_getstring(vm, -1, &unitId);
        AdManager* mgr = AdManager::getInstance();
        if(mgr) mgr->setBannerAdUnitId(unitId);
        return 0;
    }

    SQInteger MonetisationNamespace::showBannerAd(HSQUIRRELVM vm){
        AdManager* mgr = AdManager::getInstance();
        if(mgr) mgr->showBannerAd();
        return 0;
    }

    SQInteger MonetisationNamespace::hideBannerAd(HSQUIRRELVM vm){
        AdManager* mgr = AdManager::getInstance();
        if(mgr) mgr->hideBannerAd();
        return 0;
    }

    SQInteger MonetisationNamespace::setInterstitialAdUnitId(HSQUIRRELVM vm){
        const SQChar* unitId;
        sq_getstring(vm, -1, &unitId);
        AdManager* mgr = AdManager::getInstance();
        if(mgr) mgr->setInterstitialAdUnitId(unitId);
        return 0;
    }

    SQInteger MonetisationNamespace::loadInterstitialAd(HSQUIRRELVM vm){
        AdManager* mgr = AdManager::getInstance();
        if(mgr) mgr->loadInterstitialAd();
        return 0;
    }

    SQInteger MonetisationNamespace::showInterstitialAd(HSQUIRRELVM vm){
        AdManager* mgr = AdManager::getInstance();
        if(mgr) mgr->showInterstitialAd();
        return 0;
    }

    SQInteger MonetisationNamespace::isInterstitialAdReady(HSQUIRRELVM vm){
        AdManager* mgr = AdManager::getInstance();
        sq_pushbool(vm, mgr ? mgr->isInterstitialAdReady() : false);
        return 1;
    }

    void MonetisationNamespace::setupNamespace(HSQUIRRELVM vm){

        /**SQFunction
        @name setBannerAdUnitId
        @desc Set the ad unit ID for banner ads. Must be called before showBannerAd.
        @param1:unitId:string The AdMob ad unit ID (e.g. "ca-app-pub-xxx/yyy")
        */
        ScriptUtils::addFunction(vm, setBannerAdUnitId, "setBannerAdUnitId", 2, ".s");
        /**SQFunction
        @name showBannerAd
        @desc Request and display a banner ad at the bottom of the screen.
        */
        ScriptUtils::addFunction(vm, showBannerAd, "showBannerAd");
        /**SQFunction
        @name hideBannerAd
        @desc Hide the currently displayed banner ad.
        */
        ScriptUtils::addFunction(vm, hideBannerAd, "hideBannerAd");

        /**SQFunction
        @name setInterstitialAdUnitId
        @desc Set the ad unit ID for interstitial (fullscreen) ads. Must be called before loadInterstitialAd.
        @param1:unitId:string The AdMob ad unit ID (e.g. "ca-app-pub-xxx/zzz")
        */
        ScriptUtils::addFunction(vm, setInterstitialAdUnitId, "setInterstitialAdUnitId", 2, ".s");
        /**SQFunction
        @name loadInterstitialAd
        @desc Begin loading an interstitial ad. Subscribe to AD_EVENT_INTERSTITIAL_LOADED to know when ready.
        */
        ScriptUtils::addFunction(vm, loadInterstitialAd, "loadInterstitialAd");
        /**SQFunction
        @name showInterstitialAd
        @desc Present the loaded interstitial ad. Has no effect if no ad is ready.
        */
        ScriptUtils::addFunction(vm, showInterstitialAd, "showInterstitialAd");
        /**SQFunction
        @name isInterstitialAdReady
        @desc Returns true if an interstitial ad has been loaded and is ready to display.
        @returns bool
        */
        ScriptUtils::addFunction(vm, isInterstitialAdReady, "isInterstitialAdReady");
    }
}

#endif //ENABLE_ADMOB
