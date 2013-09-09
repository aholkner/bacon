#include "View.h"
#include "Platform.h"
#include "../Bacon.h"
#include "../BaconInternal.h"

#include <OpenGLES/ES2/gl.h>

@implementation View

+ (::Class)layerClass
{
    return [CAEAGLLayer class];
}

-(id)initWithFrame:(CGRect)frame
{
	if (!(self = [super initWithFrame:frame]))
		return nil;
	
    CGSize viewSize = [UIScreen mainScreen].bounds.size;
	//	self.contentScaleFactor = [UIScreen mainScreen].scale;
    
    // Get the layer
    CAEAGLLayer* eaglLayer = (CAEAGLLayer*)self.layer;
    eaglLayer.bounds = CGRectMake(0, 0, viewSize.width, viewSize.height);
    eaglLayer.opaque = YES;
    eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking,
                                    kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, (void*) nil];
	EAGLContext* context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    if (!context || ![EAGLContext setCurrentContext:context])
    {
        NSLog(@"Failed to create GLES context");
        abort();
    }
	
	GLuint renderBuffer;
	glGenRenderbuffers(1, &renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    [context renderbufferStorage:GL_RENDERBUFFER
					fromDrawable:(id<EAGLDrawable>)self.layer];
	
	GLuint frameBuffer;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderBuffer);

	glClearColor(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER];

	return self;
}

@end