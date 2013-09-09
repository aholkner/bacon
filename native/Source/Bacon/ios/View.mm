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
	context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    if (!context || ![EAGLContext setCurrentContext:context])
    {
        NSLog(@"Failed to create GLES context");
        abort();
    }
	
	glGenRenderbuffers(1, &renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    [context renderbufferStorage:GL_RENDERBUFFER
					fromDrawable:(id<EAGLDrawable>)self.layer];
	
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderBuffer);
	
	// Display link
	g_DisplayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(onDisplayLink)];
	[g_DisplayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];

	Graphics_InitGL(frameBuffer);
	
	return self;
}

static int s_LastWidth = -1, s_LastHeight = -1;

-(void)onDisplayLink
{
	CGSize size = self.frame.size;
	if (s_LastWidth != size.width || s_LastHeight != size.height)
	{
		Window_OnSizeChanged(size.width, size.height);
		s_LastWidth = size.width;
		s_LastHeight = size.height;
	}
	
	[EAGLContext setCurrentContext:context];
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	
	Graphics_BeginFrame(size.width, size.height);
	Bacon_InternalTick();
	Graphics_EndFrame();
	
	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER];
		
}

@end