#include "MacOSUtils.h"

#ifdef TARGET_APPLE_IPHONE
    #import <UIKit/UIKit.h>
    #include "Windowing/iOS/OgreMetalView.h"
#else
    #import <AppKit/NSWindow.h>
#endif

#include <SDL.h>

namespace AV
{

    unsigned long WindowContentViewHandle(const SDL_SysWMinfo &info)
    {
        #ifndef TARGET_APPLE_IPHONE
        NSWindow *window = info.info.cocoa.window;
        NSView *view = [window contentView];
        return (unsigned long)view;
        #endif

        return 0;
    }

    std::string GetApplicationSupportDirectory(){
        NSFileManager* manager = [NSFileManager defaultManager];
        NSArray* possibleURLs = [manager URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];
        if([possibleURLs count] <= 0){
            return "";
        }

        return [[[possibleURLs firstObject] path] UTF8String];
    }

    void AssignViewToSDLWindow(const SDL_SysWMinfo &info, Ogre::Window* win)
    {
        #ifdef TARGET_APPLE_IPHONE
        UIWindow * appWindow = info.info.uikit.window;
        UIViewController * rootViewController = appWindow.rootViewController;
        UIView * myView = [appWindow.subviews objectAtIndex:0];
        myView.opaque = NO;
        myView.alpha = 0.05;
        myView.userInteractionEnabled = YES;
        //Maybe destroy the view once it's been removed.

        void *uiViewPtr = 0;
        win->getCustomAttribute("UIView", &uiViewPtr);
        UIView *uiView = CFBridgingRelease(uiViewPtr);
        [uiView removeFromSuperview];
        UIViewController *myViewController = [[UIViewController alloc] init];
        myViewController.view = uiView;
        [appWindow addSubview: uiView];

        [appWindow sendSubviewToBack:uiView];
        [rootViewController addChildViewController:myViewController];

        OgreMetalView* metalView = static_cast<OgreMetalView*>(uiView);
        metalView.layerSizeDidUpdate = YES;
        #endif
    }

    NativeScreenSafeInsets GetScreenSafeAreaInsets(const SDL_SysWMinfo &info){
        NativeScreenSafeInsets insets;
        #ifdef TARGET_APPLE_IPHONE
        UIWindow * appWindow = info.info.uikit.window;
        UIEdgeInsets edgeInsets = appWindow.safeAreaInsets;
        insets.left = edgeInsets.left;
        insets.top = edgeInsets.top;
        insets.bottom = edgeInsets.bottom;
        insets.right = edgeInsets.right;
        #endif

        return insets;
    }

}

