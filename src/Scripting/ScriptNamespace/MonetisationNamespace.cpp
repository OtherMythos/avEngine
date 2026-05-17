#ifdef ENABLE_MONETISATION

#include "MonetisationNamespace.h"
#include "ScriptUtils.h"
#include "Event/Events/Event.h"

#ifdef ENABLE_ADMOB
    #include "System/Advertising/AdManager.h"
#endif

#ifdef ENABLE_MICROTRANSACTIONS
    #include "System/Microtransaction/PurchaseManager.h"
#endif

namespace AV {

#ifdef ENABLE_ADMOB
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

    SQInteger MonetisationNamespace::setPersonalisedAds(HSQUIRRELVM vm){
        SQBool enabled;
        sq_getbool(vm, -1, &enabled);
        AdManager* mgr = AdManager::getInstance();
        if(mgr) mgr->setPersonalisedAds(enabled == SQTrue);
        return 0;
    }

    SQInteger MonetisationNamespace::isPersonalisedAds(HSQUIRRELVM vm){
        AdManager* mgr = AdManager::getInstance();
        sq_pushbool(vm, mgr ? mgr->isPersonalisedAds() : true);
        return 1;
    }

    SQInteger MonetisationNamespace::getBannerAdBounds(HSQUIRRELVM vm){
        AdManager* mgr = AdManager::getInstance();

        sq_newtableex(vm, 5);  //Create table with 5 entries

        if(mgr) {
            auto bounds = mgr->getBannerAdBounds();

            //Add x
            sq_pushstring(vm, _SC("x"), -1);
            sq_pushfloat(vm, bounds.x);
            sq_newslot(vm, -3, SQFalse);

            //Add y
            sq_pushstring(vm, _SC("y"), -1);
            sq_pushfloat(vm, bounds.y);
            sq_newslot(vm, -3, SQFalse);

            //Add width
            sq_pushstring(vm, _SC("width"), -1);
            sq_pushfloat(vm, bounds.width);
            sq_newslot(vm, -3, SQFalse);

            //Add height
            sq_pushstring(vm, _SC("height"), -1);
            sq_pushfloat(vm, bounds.height);
            sq_newslot(vm, -3, SQFalse);

            //Add active
            sq_pushstring(vm, _SC("active"), -1);
            sq_pushbool(vm, bounds.active);
            sq_newslot(vm, -3, SQFalse);
        } else {
            //Still populate table even if no manager
            sq_pushstring(vm, _SC("x"), -1);
            sq_pushfloat(vm, 0.0f);
            sq_newslot(vm, -3, SQFalse);

            sq_pushstring(vm, _SC("y"), -1);
            sq_pushfloat(vm, 0.0f);
            sq_newslot(vm, -3, SQFalse);

            sq_pushstring(vm, _SC("width"), -1);
            sq_pushfloat(vm, 0.0f);
            sq_newslot(vm, -3, SQFalse);

            sq_pushstring(vm, _SC("height"), -1);
            sq_pushfloat(vm, 0.0f);
            sq_newslot(vm, -3, SQFalse);

            sq_pushstring(vm, _SC("active"), -1);
            sq_pushbool(vm, false);
            sq_newslot(vm, -3, SQFalse);
        }

        return 1;
    }
#endif //ENABLE_ADMOB

#ifdef ENABLE_MICROTRANSACTIONS
    SQInteger MonetisationNamespace::purchaseProduct(HSQUIRRELVM vm){
        const SQChar* productId;
        sq_getstring(vm, -1, &productId);
        PurchaseManager* mgr = PurchaseManager::getInstance();
        if(mgr) mgr->purchaseProduct(productId);
        return 0;
    }

    SQInteger MonetisationNamespace::restorePurchases(HSQUIRRELVM vm){
        PurchaseManager* mgr = PurchaseManager::getInstance();
        if(mgr) mgr->restorePurchases();
        return 0;
    }

    SQInteger MonetisationNamespace::queryProductInfo(HSQUIRRELVM vm){
        const SQChar* productId;
        sq_getstring(vm, -1, &productId);
        PurchaseManager* mgr = PurchaseManager::getInstance();
        if(mgr) mgr->queryProductInfo(productId);
        return 0;
    }

    SQInteger MonetisationNamespace::canMakePayments(HSQUIRRELVM vm){
        //On platforms without purchase support the manager is null, so return false.
        sq_pushbool(vm, PurchaseManager::getInstance() != nullptr);
        return 1;
    }
#endif //ENABLE_MICROTRANSACTIONS

    void MonetisationNamespace::setupNamespace(HSQUIRRELVM vm){

#ifdef ENABLE_ADMOB
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
        @desc Begin loading an interstitial ad. Subscribe to _EVENT_ADVERTISING_INTERSTITIAL_LOADED to know when ready.
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
        /**SQFunction
        @name getBannerAdBounds
        @desc Get the bounds of the banner ad in physical screen pixels.
        @returns table with keys: x, y, width, height, active
        */
        ScriptUtils::addFunction(vm, getBannerAdBounds, "getBannerAdBounds");
        /**SQFunction
        @name setPersonalisedAds
        @desc Enable or disable personalised ads. When disabled, requests non-personalised ads from Google Mobile Ads. Default is true. Applies to future ad requests.
        @param1:enabled:bool True to enable personalised ads, false for non-personalised ads only
        */
        ScriptUtils::addFunction(vm, setPersonalisedAds, "setPersonalisedAds", 2, ".b");
        /**SQFunction
        @name isPersonalisedAds
        @desc Returns whether personalised ads are currently enabled.
        @returns bool
        */
        ScriptUtils::addFunction(vm, isPersonalisedAds, "isPersonalisedAds");
#endif //ENABLE_ADMOB

#ifdef ENABLE_MICROTRANSACTIONS
        /**SQFunction
        @name purchaseProduct
        @desc Initiate a purchase for the given product ID. Subscribe to _EVENT_PURCHASE_PRODUCT_PURCHASED or _EVENT_PURCHASE_PRODUCT_FAILED for the result.
        @param1:productId:string The platform product identifier (e.g. "com.example.coins100")
        */
        ScriptUtils::addFunction(vm, purchaseProduct, "purchaseProduct", 2, ".s");
        /**SQFunction
        @name restorePurchases
        @desc Restore previously completed purchases. Fires _EVENT_PURCHASE_PRODUCT_PURCHASED per product, then _EVENT_PURCHASE_RESTORE_COMPLETED or _EVENT_PURCHASE_RESTORE_FAILED.
        */
        ScriptUtils::addFunction(vm, restorePurchases, "restorePurchases");
        /**SQFunction
        @name queryProductInfo
        @desc Request localised product info for the given product ID. Subscribe to _EVENT_PURCHASE_PRODUCT_INFO for the result.
        @param1:productId:string The platform product identifier
        */
        ScriptUtils::addFunction(vm, queryProductInfo, "queryProductInfo", 2, ".s");
        /**SQFunction
        @name canMakePayments
        @desc Returns true if the platform purchase manager is available and payments can be made.
        @returns bool
        */
        ScriptUtils::addFunction(vm, canMakePayments, "canMakePayments");
#endif //ENABLE_MICROTRANSACTIONS
    }
}

#endif //ENABLE_MONETISATION
