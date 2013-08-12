#pragma once

extern NSWindow* g_Window;
extern NSString* g_WindowTitle;
extern NSRect g_WindowStartFrame;
extern bool g_WindowResizable;
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

- (void)displayLinkCallback;
@end

extern View* g_View;

void Window_Create();