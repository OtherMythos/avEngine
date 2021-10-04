#ifndef __MACOS__H
#define __MACOS__H

#include "Logger/Log.h"
#include "System/SystemSetup/SystemSetup.h"

#import "Foundation/Foundation.h"
#import <UIKit/UIKit.h>

#include "OgreWindow.h"

#include "System/Base.h"
@interface GameViewController : UIViewController

@end

@interface AppDelegate : NSObject <UIApplicationDelegate>
{
    //CADisplayLink *mDisplayLink;
    //NSDate* mDate;
    //NSTimeInterval mLastFrameTime;
    //Base *base;
    UIWindow *window;
    GameViewController *viewController;
}

//@property (nonatomic) NSTimeInterval mLastFrameTime;
//@property (strong, nonatomic)
@property (strong, nonatomic) UIWindow *window;

@end

@implementation AppDelegate

- (void)go {

}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    AV::Log::Init();

    window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
    viewController = [[GameViewController alloc] init];
    viewController.view.backgroundColor = UIColor.redColor;

    [window setRootViewController:viewController];
    [window makeKeyAndVisible];

    return YES;
}

- (void)applicationWillTerminate:(UIApplication *)application
{

}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    NSLog(@"Active bitches!");


}

- (void)applicationWillResignActive:(UIApplication *)application
{
}

- (void)renderOneFrame:(id)sender
{

}

@end

@implementation GameViewController{
    double _accumulator;
    CADisplayLink *_timer;

    double _timeSinceLast;
    CFTimeInterval _startTime;

    AV::Base *base;
}

-(void)viewDidLoad{
    [super viewDidLoad];

    const std::vector<std::string> args;
    AV::SystemSetup::setup(args);
    base = new AV::Base();

    /*
    Ogre::Window *renderWindow = base->getRenderWindow();
    void *uiViewPtr = 0;
    renderWindow->getCustomAttribute("UIView", &uiViewPtr);
    UIView *uiView = CFBridgingRelease(uiViewPtr);
    self.view = uiView;
    */

    NSLog(@"loading view");
}

-(void)viewWillAppear:(BOOL)animated{
    [super viewWillAppear:animated];

    NSLog(@"Going to appear");
//}

//- (void)applicationDidBecomeActive:(UIApplication *)application{
//- (void)sceneDidBecomeActive:(UIScene *)application{
    NSLog(@"Did become active");

    //Create the timer required by Metal. iOS will call us at fixed intervals.
    if( _timer )
    {
        [_timer invalidate];
        _timer = nullptr;
    }
    // create a game loop timer using a display link
    //_timer = [[UIScreen mainScreen] displayLinkWithTarget:self
    //                                             selector:@selector(mainLoop:)];
    //_timer = [CADisplayLink displayLinkWithTarget:self selector:@selector(mainLoop:)];
    _timer = [CADisplayLink displayLinkWithTarget:self selector:@selector(mainLoop)];
    //_timer.preferredFramesPerSecond = 60;
    _timer.frameInterval = 1;
    [_timer addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];

    _timeSinceLast = 1.0 / 60.0;
    _startTime = CACurrentMediaTime();
}

//-(void)mainLoop:(CADisplayLink*)sender{
-(void)mainLoop{
    CFTimeInterval endTime = CACurrentMediaTime();
    _timeSinceLast = endTime - _startTime;
    _timeSinceLast = std::min( 1.0, _timeSinceLast ); //Prevent from going haywire.
    _startTime = endTime;

    base->update();
}

@end


#endif
