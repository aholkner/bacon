#include "View.h"
#include "Platform.h"
#include "../Bacon.h"
#include "../BaconInternal.h"

#include <OpenGLES/ES2/gl.h>


struct Touch
{
	float m_X, m_Y;
	bool m_Pressed;
};
const int MaxTouches = 11;
Touch s_Touches[MaxTouches];


@implementation View

+ (::Class)layerClass
{
    return [CAEAGLLayer class];
}

-(id)initWithFrame:(CGRect)frame
{
	if (!(self = [super initWithFrame:frame]))
		return nil;
	
	self.multipleTouchEnabled = YES;
	for (int i = 0; i < MaxTouches; ++i)
	{
		s_Touches[i].m_X = 0.f;
		s_Touches[i].m_Y = 0.f;
		s_Touches[i].m_Pressed = false;
	}
	
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

#pragma mark Touch

static int GetAvailableTouchIndex()
{
	for (int i = 0; i < MaxTouches; ++i)
	{
		if (!s_Touches[i].m_Pressed)
			return i;
	}
	return -1;
}

static int GetTouchIndex(UIView* view, UITouch* touch)
{
	CGPoint p = [touch previousLocationInView:view];

	int best = -1;
	float bestDistance = FLT_MAX;
	for (int i = 0; i < MaxTouches; ++i)
	{
		if (!s_Touches[i].m_Pressed)
			continue;
		float dx = p.x - s_Touches[i].m_X;
		float dy = p.y - s_Touches[i].m_Y;
		float distance = dx * dx + dy * dy;
		if (distance < bestDistance)
		{
			best = i;
			bestDistance = distance;
		}
	}
	
	return best;
}

static void UpdateTouchStates(UIView* view, NSSet* touches, int state)
{
	for (UITouch* touch : touches)
	{
		int i = GetTouchIndex(view, touch);
		if (i != -1)
		{
			CGPoint p = [touch locationInView:view];
			s_Touches[i].m_X = p.x;
			s_Touches[i].m_Y = p.y;
			s_Touches[i].m_Pressed = state == Bacon_Touch_State_Pressed;
			Touch_SetTouchState(i, state, p.x, p.y);
		}
	}
}

-(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	for (UITouch* touch : touches)
	{
		CGPoint p = [touch locationInView:self];

		int i = GetAvailableTouchIndex();
		if (i != -1)
		{
			s_Touches[i].m_Pressed = true;
			s_Touches[i].m_X = p.x;
			s_Touches[i].m_Y = p.y;
			Touch_SetTouchState(i, Bacon_Touch_State_Pressed, p.x, p.y);
		}
	}
}

-(void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	UpdateTouchStates(self, touches, Bacon_Touch_State_Cancelled);
}

-(void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	UpdateTouchStates(self, touches, Bacon_Touch_State_Released);
}

-(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	UpdateTouchStates(self, touches, Bacon_Touch_State_Pressed);
}

@end