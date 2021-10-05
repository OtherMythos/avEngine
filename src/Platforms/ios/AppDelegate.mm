#import "AppDelegate.h"

#import "ViewController.h"

@implementation AppDelegate

ViewController *viewController;

//- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
//    return YES;
//}


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
    viewController = [[ViewController alloc] init];
    //viewController.view.backgroundColor = UIColor.redColor;

    [window setRootViewController:viewController];
    [window makeKeyAndVisible];

    return YES;
}


@end
