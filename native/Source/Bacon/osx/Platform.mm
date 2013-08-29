#include "../Bacon.h"
#include "../BaconInternal.h"
#include "Platform.h"

#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

NSWindow* g_Window = nil;
NSString* g_WindowTitle = @"Bacon";
NSRect g_WindowStartFrame = NSMakeRect(0, 0, 640, 480);
bool g_WindowResizable = false;
bool g_WindowStartFullscreen = false;
bool g_MakeFirstResponder = false;

View* g_View = nil;
NSOpenGLContext* g_Context;
float g_Width, g_Height;
CVDisplayLinkRef g_DisplayLink;
CGLPixelFormatObj g_PixelFormat;

static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
	[g_View performSelectorOnMainThread:@selector(displayLinkCallback) withObject:Nil waitUntilDone:NO];
    return kCVReturnSuccess;
}

static void LogSystemInfo()
{
	NSProcessInfo* pi = [NSProcessInfo processInfo];
	Bacon_Log(Bacon_LogLevel_Info, "OS X %s", pi.operatingSystemVersionString.UTF8String);
	Bacon_Log(Bacon_LogLevel_Info, "Number of processors: %d", pi.processorCount);
}

static uint64_t s_PerformanceStartTime;
static float s_PerformanceTimebase;

void Platform_Init()
{
	s_PerformanceStartTime = mach_absolute_time();
	mach_timebase_info_data_t timebase;
	mach_timebase_info(&timebase);
	s_PerformanceTimebase = (float)timebase.numer / timebase.denom / 1000000000.f;

	if ([[NSScreen mainScreen] respondsToSelector:@selector(backingScaleFactor)])
		Window_SetDeviceContentScale([NSScreen mainScreen].backingScaleFactor);
}

void Platform_Shutdown()
{ }

void Platform_GetPerformanceTime(float& time)
{
	uint64_t elapsed = mach_absolute_time() - s_PerformanceStartTime;
	time = elapsed * s_PerformanceTimebase;
}

int Platform_Run()
{
	// Minimal Cocoa startup
	// http://www.cocoawithlove.com/2010/09/minimalist-cocoa-programming.html
	NSAutoreleasePool* pool = [NSAutoreleasePool new];
    [BaconApplication sharedApplication];
	[BaconApplication sharedApplication].delegate = (BaconApplication*)[BaconApplication sharedApplication];
	
	LogSystemInfo();
	
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    id menubar = [[NSMenu new] autorelease];
    id appMenuItem = [[NSMenuItem new] autorelease];
    [menubar addItem:appMenuItem];
    [NSApp setMainMenu:menubar];
    id appMenu = [[NSMenu new] autorelease];
    id quitTitle = [@"Quit " stringByAppendingString:g_WindowTitle];
    id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle
												  action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];

	NSRect frame = g_WindowStartFrame;
	
	int styleMask = NSTitledWindowMask |  NSClosableWindowMask |NSMiniaturizableWindowMask;
	if (g_WindowResizable)
		styleMask |= NSResizableWindowMask;
	
    g_Window = [[NSWindow alloc] initWithContentRect:frame
											styleMask:styleMask
											  backing:NSBackingStoreBuffered
												defer:NO];

	[g_Window center];
    [g_Window setTitle:g_WindowTitle];

	g_View = [View alloc];
	[g_View initWithFrame:frame];
	[g_View setNeedsDisplay:YES];
	[g_Window setContentView:g_View];
    [g_Window makeKeyAndOrderFront:nil];
	if (g_WindowStartFullscreen)
		[g_View enterFullScreenMode:[NSScreen mainScreen]
						withOptions:nil];
	
	// Synchronize buffer swaps with vertical refresh rate
    GLint swapInt = 1;
    [g_Context setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
	
    // Create a display link capable of being used with all active displays
    CVDisplayLinkCreateWithActiveCGDisplays(&g_DisplayLink);
	
    // Set the renderer output callback function
    CVDisplayLinkSetOutputCallback(g_DisplayLink, &MyDisplayLinkCallback, g_View);
	
    // Set the display link for the current renderer
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(g_DisplayLink, (CGLContextObj)[g_Context CGLContextObj], [g_View CGLPixelFormatObj]);
	
    // Activate the display link
    CVDisplayLinkStart(g_DisplayLink);
	
	[NSApp activateIgnoringOtherApps:YES];
    [NSApp run];
	[pool release];
	return 0;
}

void Platform_Stop()
{
	[NSApp terminate:nil];
}