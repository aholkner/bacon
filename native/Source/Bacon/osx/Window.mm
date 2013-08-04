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

@end

int Bacon_SetWindowSize(int width, int height)
{
	g_WindowStartFrame = NSMakeRect(0, 0, width, height);
	if (g_View)
		[g_View setFrameSize:g_WindowStartFrame.size];
	if (g_Window)
		[g_Window setContentSize:g_WindowStartFrame.size];
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
	