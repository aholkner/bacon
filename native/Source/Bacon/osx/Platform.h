#pragma once

extern NSWindow* g_Window;
extern NSRect g_WindowStartFrame;
extern bool g_WindowStartFullscreen;
extern bool g_MakeFirstResponder;
extern NSOpenGLContext* g_Context;

@interface BaconApplication : NSApplication
{ }
@end

@interface View : NSView
{
	CGLPixelFormatObj m_PixelFormat;
}
@property (readonly) CGLPixelFormatObj CGLPixelFormatObj;
@end

extern View* g_View;