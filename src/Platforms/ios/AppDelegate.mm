#import "AppDelegate.h"

#import "ViewController.h"

@implementation AppDelegate

ViewController *viewController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state.
    // This can occur for certain types of temporary interruptions (such as an incoming phone
    // call or SMS message) or when the user quits the application and it begins the transition
    // to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates.
    // Games should use this method to pause the game.

    //Do this in order to get GameViewController::viewWillDisappear called and thus
    //stop the timer (otherwise we'll get unable to get Metal Drawable errors).
    //There are multiple ways to do this. This is one of them.
    [self.window.rootViewController viewWillDisappear:NO];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive.
    // If the application was previously in the background, optionally refresh the user interface.

    //Do this in order to get GameViewController::viewWillAppear get called and
    //restore the timer. There are multiple ways to do this. This is one of them.
    [self.window.rootViewController viewWillAppear:NO];
}

@end
