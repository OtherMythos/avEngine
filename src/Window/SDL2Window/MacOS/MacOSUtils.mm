#include "MacOSUtils.h"

//#import <AppKit/NSWindow.h>
#import <UIKit/UIKit.h>
#include <SDL.h>

#include "Windowing/iOS/OgreMetalView.h"

namespace AV
{

    unsigned long WindowContentViewHandle(const SDL_SysWMinfo &info)
    {
        #ifndef TARGET_OS_IPHONE
        NSWindow *window = info.info.cocoa.window;
        NSView *view = [window contentView];
        return (unsigned long)view;
        #endif

        return 0;
    }

    void AssignViewToSDLWindow(const SDL_SysWMinfo &info, Ogre::Window* win)
    {
        #ifdef TARGET_OS_IPHONE
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

}

