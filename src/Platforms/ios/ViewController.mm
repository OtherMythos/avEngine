#import "ViewController.h"

#include "Logger/Log.h"
#include "System/SystemSetup/SystemSetup.h"
#include "System/Base.h"

#include "Window/Window.h"
#include "OgreWindow.h"

@implementation ViewController

AV::Base *base;

- (void)viewDidLoad {
    [super viewDidLoad];

    UIScreen *mainScreen = [UIScreen mainScreen];
    CGRect screenRect = [mainScreen bounds];
    float width = screenRect.size.width;
    float height = screenRect.size.height;
    //375, 667


    AV::Log::Init();

    const std::vector<std::string> args;
    AV::SystemSetup::setup(args);
    base = new AV::Base();

    //Setup the window.
    Ogre::Window *renderWindow = base->getWindow()->getRenderWindow();
    void *uiViewPtr = 0;
    renderWindow->getCustomAttribute("UIView", &uiViewPtr);
    UIView *uiView = CFBridgingRelease(uiViewPtr);
    self.view = uiView;
}

- (BOOL)prefersStatusBarHidden {
    return YES;
}

//- (void)viewDidAppear:(BOOL)animated
- (void)viewWillAppear:(BOOL)animated
{

//    [super viewDidAppear:animated];
    [super viewWillAppear:animated];



    if (true)
    {
        self.displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(displayLinkDidFire:)];
        [self.displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
    }
    else
    {
        [self.displayLink invalidate];
        self.displayLink = nil;
    }
}

- (void)displayLinkDidFire:(CADisplayLink *)displayLink
{
    base->update();
}

@end
