#ifdef ENABLE_ADMOB
#ifdef TARGET_APPLE_IPHONE

#include "iosAdManager.h"

#import <GoogleMobileAds/GoogleMobileAds.h>
#import <UIKit/UIKit.h>
#include "SDL.h"
#include "SDL_syswm.h"
#include "Event/Events/AdvertisingEvent.h"
#include "Event/EventDispatcher.h"

namespace AV {}

//----------------------------------------------------------------------------
//Objective-C delegate that receives GAD callbacks and forwards them to C++.
//----------------------------------------------------------------------------
@interface iOSAdDelegate : NSObject <GADBannerViewDelegate, GADFullScreenContentDelegate>
@property (nonatomic, assign) AV::iosAdManager* owner;
@property (nonatomic, strong) GADBannerView* bannerView;
@property (nonatomic, strong) GADInterstitialAd* interstitialAd;
@end

@implementation iOSAdDelegate

- (void)bannerViewDidReceiveAd:(GADBannerView*)bannerView {
    AV::AdvertisingEventBannerLoaded event;
    AV::EventDispatcher::transmitEvent(AV::EventType::System, event);
}

- (void)bannerView:(GADBannerView*)bannerView didFailToReceiveAdWithError:(NSError*)error {
    AV::AdvertisingEventBannerFailed event;
    AV::EventDispatcher::transmitEvent(AV::EventType::System, event);
}

- (void)adDidDismissFullScreenContent:(id<GADFullScreenPresentingAd>)ad {
    if(self.owner) {
        self.owner->mInterstitialReady = false;
        self.interstitialAd = nil;
    }
    AV::AdvertisingEventInterstitialClosed event;
    AV::EventDispatcher::transmitEvent(AV::EventType::System, event);
}

- (void)ad:(id<GADFullScreenPresentingAd>)ad didFailToPresentFullScreenContentWithError:(NSError*)error {
    if(self.owner) {
        self.owner->mInterstitialReady = false;
        self.interstitialAd = nil;
    }
    AV::AdvertisingEventInterstitialFailed event;
    AV::EventDispatcher::transmitEvent(AV::EventType::System, event);
}

@end

//----------------------------------------------------------------------------
//Helpers to get the SDL UIViewController and UIView.
//----------------------------------------------------------------------------
static UIViewController* _getSDLViewController() {
    SDL_Window* sdlWindow = SDL_GL_GetCurrentWindow();
    if(sdlWindow) {
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        if(SDL_GetWindowWMInfo(sdlWindow, &wmInfo) == SDL_TRUE) {
            return wmInfo.info.uikit.window.rootViewController;
        }
    }

    //Fallback: Use the key window from UIApplication.
    UIWindow* keyWindow = nullptr;
    if(@available(iOS 13.0, *)) {
        for(UIWindowScene* windowScene in UIApplication.sharedApplication.connectedScenes) {
            if(windowScene.activationState == UISceneActivationStateForegroundActive) {
                keyWindow = windowScene.windows.firstObject;
                if(keyWindow) break;
            }
        }
    } else {
        keyWindow = UIApplication.sharedApplication.keyWindow;
    }

    if(keyWindow) {
        return keyWindow.rootViewController;
    }

    return nil;
}

//----------------------------------------------------------------------------
//C++ implementation
//----------------------------------------------------------------------------
namespace AV {

    iosAdManager::iosAdManager() {
        iOSAdDelegate* delegate = [[iOSAdDelegate alloc] init];
        delegate.owner = this;
        mDelegate = (__bridge_retained void*)delegate;
    }

    iosAdManager::~iosAdManager() {
        if(mDelegate) {
            //Release the retained delegate.
            iOSAdDelegate* delegate = (__bridge_transfer iOSAdDelegate*)mDelegate;
            (void)delegate;
            mDelegate = nullptr;
        }
    }

    void iosAdManager::initialise() {
        [[GADMobileAds sharedInstance] startWithCompletionHandler:nil];
    }

    void iosAdManager::setBannerAdUnitId(const std::string& unitId) {
        mBannerUnitId = unitId;
    }

    void iosAdManager::showBannerAd() {
        if(mBannerUnitId.empty()) return;

        iOSAdDelegate* delegate = (__bridge iOSAdDelegate*)mDelegate;
        UIViewController* vc = _getSDLViewController();
        if(!vc) return;

        if(!delegate.bannerView) {
            delegate.bannerView = [[GADBannerView alloc] initWithAdSize:GADAdSizeBanner];
            delegate.bannerView.adUnitID = [NSString stringWithUTF8String:mBannerUnitId.c_str()];
            delegate.bannerView.rootViewController = vc;
            delegate.bannerView.delegate = delegate;

            //Position the banner at the bottom of the screen.
            //Add directly to the app window to ensure it's on top of the Metal view.
            UIWindow* appWindow = vc.view.window;
            if(!appWindow) {
                //Fallback to adding to the view controller's view if we can't get the window.
                appWindow = [UIApplication sharedApplication].keyWindow;
            }

            delegate.bannerView.translatesAutoresizingMaskIntoConstraints = NO;
            [appWindow addSubview:delegate.bannerView];
            [appWindow bringSubviewToFront:delegate.bannerView];

            [NSLayoutConstraint activateConstraints:@[
                [delegate.bannerView.centerXAnchor constraintEqualToAnchor:appWindow.centerXAnchor],
                [delegate.bannerView.bottomAnchor constraintEqualToAnchor:appWindow.safeAreaLayoutGuide.bottomAnchor]
            ]];
        }

        delegate.bannerView.hidden = NO;
        [delegate.bannerView.window bringSubviewToFront:delegate.bannerView];
        GADRequest* request = [GADRequest request];
        [delegate.bannerView loadRequest:request];
        mBannerVisible = true;
    }

    void iosAdManager::hideBannerAd() {
        iOSAdDelegate* delegate = (__bridge iOSAdDelegate*)mDelegate;
        if(delegate.bannerView) {
            delegate.bannerView.hidden = YES;
        }
        mBannerVisible = false;
    }

    void iosAdManager::setInterstitialAdUnitId(const std::string& unitId) {
        mInterstitialUnitId = unitId;
    }

    void iosAdManager::loadInterstitialAd() {
        if(mInterstitialUnitId.empty()) return;

        iOSAdDelegate* delegate = (__bridge iOSAdDelegate*)mDelegate;
        NSString* unitId = [NSString stringWithUTF8String:mInterstitialUnitId.c_str()];
        GADRequest* request = [GADRequest request];
        iosAdManager* self = this;

        [GADInterstitialAd loadWithAdUnitID:unitId
                                    request:request
                          completionHandler:^(GADInterstitialAd* ad, NSError* error) {
            if(error) {
                self->mInterstitialReady = false;
                AV::AdvertisingEventInterstitialFailed failureEvent;
                AV::EventDispatcher::transmitEvent(AV::EventType::System, failureEvent);
                return;
            }
            delegate.interstitialAd = ad;
            delegate.interstitialAd.fullScreenContentDelegate = delegate;
            self->mInterstitialReady = true;
            AV::AdvertisingEventInterstitialLoaded loadedEvent;
            AV::EventDispatcher::transmitEvent(AV::EventType::System, loadedEvent);
        }];
    }

    void iosAdManager::showInterstitialAd() {
        if(!mInterstitialReady) return;

        iOSAdDelegate* delegate = (__bridge iOSAdDelegate*)mDelegate;
        UIViewController* vc = _getSDLViewController();
        if(!vc || !delegate.interstitialAd) return;

        [delegate.interstitialAd presentFromRootViewController:vc];
    }

    BannerAdBounds iosAdManager::getBannerAdBounds() const {
        BannerAdBounds bounds;
        bounds.active = mBannerVisible;

        if(!mBannerVisible) {
            return bounds;  //Return zero bounds if banner not visible
        }

        iOSAdDelegate* delegate = (__bridge iOSAdDelegate*)mDelegate;
        if(delegate && delegate.bannerView && !delegate.bannerView.hidden) {
            //Ensure layout is complete
            UIWindow* appWindow = delegate.bannerView.window;
            if(appWindow) {
                [appWindow layoutIfNeeded];
            }

            //Get the banner view's frame in its window's coordinate system
            CGRect frameInWindow = delegate.bannerView.frame;

            //Convert from window coordinates to screen coordinates
            UIWindow* window = delegate.bannerView.window;
            if(window) {
                CGRect screenRect = [window convertRect:frameInWindow toWindow:nil];

                //Account for screen scale
                CGFloat screenScale = [[UIScreen mainScreen] scale];
                bounds.x = screenRect.origin.x;
                bounds.y = screenRect.origin.y;
                bounds.width = screenRect.size.width;
                bounds.height = screenRect.size.height;
            } else {
                //Fallback if we can't get the window
                CGFloat screenScale = [[UIScreen mainScreen] scale];
                bounds.x = frameInWindow.origin.x;
                bounds.y = frameInWindow.origin.y;
                bounds.width = frameInWindow.size.width;
                bounds.height = frameInWindow.size.height;
            }
            bounds.active = true;
        }

        return bounds;
    }

    bool iosAdManager::isInterstitialAdReady() const {
        return mInterstitialReady;
    }
}

#endif //TARGET_APPLE_IPHONE
#endif //ENABLE_ADMOB
