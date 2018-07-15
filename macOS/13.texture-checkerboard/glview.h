#import <Cocoa/Cocoa.h>

#import <QuartzCore/CVDisplayLink.h>

#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>

#import "vmath.h"

// 'C' style global declaration
CVReturn MyDisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp *, const CVTimeStamp *,
                      CVOptionFlags, CVOptionFlags *, void *);
extern FILE *gpFile;

enum 
{
	VDG_ATTRIBUTE_VERTEX = 0,
	VDG_ATTRIBUTE_COLOR,
	VDG_ATTRIBUTE_NORMAL,
	VDG_ATTRIBUTE_TEXTURE
};

@interface GLView : NSOpenGLView

@end
