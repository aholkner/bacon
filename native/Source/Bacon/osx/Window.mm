#include "../Bacon.h"
#include "../BaconInternal.h"
#include "Platform.h"

@implementation BaconApplication

-(void)sendEvent:(NSEvent *)anEvent
{
	// Workaround NSApplication swallowing keyUp events for keys when command modifier is active
	if ([anEvent type] == NSKeyUp && ([anEvent modifierFlags] & NSCommandKeyMask))
		[[self keyWindow] sendEvent:anEvent];
	else
		[super sendEvent:anEvent];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

@end

void Window_Create()
{
	NSRect frame = g_WindowStartFrame;
	
	g_View = [View alloc];
	[g_View initWithFrame:frame];
	[g_View setNeedsDisplay:YES];
	
	int styleMask = NSTitledWindowMask |  NSClosableWindowMask |NSMiniaturizableWindowMask | NSResizableWindowMask;
	
    g_Window = [[NSWindow alloc] initWithContentRect:frame
										   styleMask:styleMask
											 backing:NSBackingStoreBuffered
											   defer:NO];
	
	//    [window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
    [g_Window setTitle:g_WindowTitle];
	[g_Window setContentView:g_View];
    [g_Window makeKeyAndOrderFront:nil];
	if (g_WindowStartFullscreen)
		[g_View enterFullScreenMode:[NSScreen mainScreen]
						withOptions:nil];
}

int Bacon_SetWindowSize(int width, int height)
{
	g_WindowStartFrame = NSMakeRect(0, 0, width, height);
	if (g_View)
		[g_View setFrameSize:g_WindowStartFrame.size];
	if (g_Window)
		[g_Window setContentSize:g_WindowStartFrame.size];
	return Bacon_Error_None;
}

int Bacon_SetWindowTitle(const char* title)
{
	g_WindowTitle = [NSString stringWithUTF8String:title];
	if (g_Window)
		[g_Window setTitle:g_WindowTitle];
	return Bacon_Error_None;
}

int Bacon_SetWindowResizable(int resizable)
{
	g_WindowResizable = resizable;
	if (g_Window)
	{
		if (resizable)
			g_Window.styleMask |= NSResizableWindowMask;
		else
			g_Window.styleMask &= ~NSResizableWindowMask;
	}
	return Bacon_Error_None;
}

int Bacon_SetWindowFullscreen(int fullscreen)
{
	g_WindowStartFullscreen = fullscreen;
	if (g_Window &&
		fullscreen != [g_View isInFullScreenMode])
	{
		if (fullscreen)
			[g_View enterFullScreenMode:[NSScreen mainScreen] withOptions:nil];
		else
		{
			[g_View exitFullScreenModeWithOptions:nil];
			g_MakeFirstResponder = true;
		}
	}
	return Bacon_Error_None;
}
	