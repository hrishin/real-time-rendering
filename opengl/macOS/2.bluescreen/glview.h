#import <Cocoa/Cocoa.h>

#import <QuartzCore/CVDisplayLink.h>

#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>

// 'C' style global declaration
CVReturn MyDisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp *, const CVTimeStamp *,
                      CVOptionFlags, CVOptionFlags *, void *);
extern FILE *gpFile;

@interface GLView : NSOpenGLView

@end
