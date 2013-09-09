#include "AppDelegate.h"
#include "Platform.h"
#include "ViewController.h"
#include "View.h"

UIWindow* g_DeviceWindow;
UIView* g_View;
UIViewController* g_ViewController;

@implementation AppDelegate

+(void)staticInit
{
}

-(BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	g_DeviceWindow = [[UIWindow alloc] initWithFrame: [UIScreen mainScreen].bounds];
    [g_DeviceWindow makeKeyAndVisible];
    g_DeviceWindow.hidden = NO;
	
	UIScreen* screen = [UIScreen mainScreen];
	g_View = [[View alloc] initWithFrame:screen.bounds];
	g_View.autoresizesSubviews = NO;
	
	g_ViewController = [[ViewController alloc] init];
	g_ViewController.view = g_View;
	g_DeviceWindow.rootViewController = g_ViewController;
	
	[g_DeviceWindow addSubview: g_View];
	[g_DeviceWindow bringSubviewToFront:g_View];
	
	return YES;
}

@end