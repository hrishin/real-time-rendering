// headers
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#import "appdelegate.h"
#import "glview.h"

// Global varible
FILE *gpFile = NULL;

int main(int argc, const char *argv[]) {
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];
	
	NSApp = [NSApplication sharedApplication];
	
	[NSApp setDelegate: [[AppDelegate alloc]init]];
	
	[NSApp run];
	
	[pPool release];

	return (0);
}

