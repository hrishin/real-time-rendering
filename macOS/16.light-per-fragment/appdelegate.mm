#import "appdelegate.h"

// AppDelegate implementation
@implementation AppDelegate
{
@private
    NSWindow *window;
    GLView *glView;
}

- (void) applicationDidFinishLaunching : (NSNotification *) aNotification
{
    NSBundle *mainBundle = [NSBundle mainBundle];
    NSString *appDirName = [mainBundle bundlePath];
    NSString *parentDirPath = [appDirName stringByDeletingLastPathComponent];
    NSString * logFileNameWithPath = [NSString stringWithFormat: @"%@/Log.txt", parentDirPath];
    
    const char *pszLogFileNameWithPath =[logFileNameWithPath cStringUsingEncoding: NSASCIIStringEncoding];
    gpFile = fopen(pszLogFileNameWithPath, "w");
    if(gpFile == NULL)
    {
        printf("Can not create log file.\n Exiting\n");
        [self release];
        [NSApp terminate:self];
    }
    fprintf(gpFile, "Program is started successfully\n");
    
	// window 
	NSRect win_rect;
	win_rect = NSMakeRect(0.0, 0.0, 800.0, 600.0);

	// create window
	window = [[NSWindow alloc] initWithContentRect: win_rect 
			styleMask: NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskResizable
			backing: NSBackingStoreBuffered 
			defer: NO];

	[window setTitle: @"macOS OpenGL : Light per fragment on Sphere"];
	[window center];

	glView = [[GLView alloc] initWithFrame: win_rect];
	
    [window setContentView: glView];
	[window setDelegate: self];
	[window makeKeyAndOrderFront: self];
}

- (void) applicationWillTerminate: (NSNotification*) notification
{
    fprintf(gpFile, "Program is terminated successfully\n");
    if(gpFile)
    {
        fclose(gpFile);
        gpFile = NULL;
    }
}

- (void) windowWillClose: (NSNotification*) notification
{
	[NSApp terminate: self];
}

- (void) dealloc
{
	[glView release];

	[window release];

	[super dealloc];
}
@end

