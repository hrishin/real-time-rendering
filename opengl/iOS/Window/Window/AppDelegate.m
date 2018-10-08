//
//  AppDelegate.m
//  Window
//
//  Created by Hrishikesh Shinde on 22/08/18.
//

#import "AppDelegate.h"

#import "ViewController.h"

#import "CustomView.h"

@implementation AppDelegate
{
@private
    UIWindow *mainWindow;
    ViewController *mainViewController;
    CustomView *mainView;
}

- (BOOL)application: (UIApplication *)application didFinishLaunchingWithOptions: (NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    
    NSLog(@"Staring application");
    // screen bounds for fullscreen
    CGRect screenBounds = [[UIScreen mainScreen] bounds];
    
    // main window
    mainWindow = [[UIWindow alloc] initWithFrame: screenBounds];
    
    // main viewcontroller
    mainViewController = [[ViewController alloc] init];
    
    [mainWindow setRootViewController: mainViewController];
    
    mainView = [[CustomView alloc] initWithFrame: screenBounds];
    
    // increases mainView retain count by 1 more time
    [mainViewController setView: mainView];
    
    // hence redduce it by 1 more time
    // (bad deasign or use Auto reference counting)
    [mainView release];
    
    // add the ViewController's view as subview to the window
    [mainWindow addSubview: [mainViewController view]];
    
    [mainWindow makeKeyAndVisible];
    
    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}


- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}


- (void) dealloc
{
    [mainView release];
    
    [mainViewController release];
    
    [mainWindow release];
    
    [super dealloc];
}
@end
