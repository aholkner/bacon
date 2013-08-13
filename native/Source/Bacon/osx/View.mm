#include "Platform.h"
#include "../Bacon.h"
#include "../BaconInternal.h"

// For Events.h (virtual key codes).  Does not link against Carbon
#include <Carbon/Carbon.h>

#include <tr1/unordered_map>
using namespace std::tr1;

unordered_map<unichar, int> s_CharMap;
unordered_map<int, int> s_VKeyCodeMap;
static void InitKeyMap();

@implementation View

- (id)initWithFrame:(NSRect)frame
{
	InitKeyMap();
	
    self = [super initWithFrame:frame];
    if (self) {
		// Accept mouseMoved events
		NSTrackingArea *trackingArea = [[NSTrackingArea alloc] initWithRect:[self frame]
																	options:NSTrackingMouseMoved | NSTrackingActiveInKeyWindow |
																		NSTrackingInVisibleRect
																	  owner:self
																   userInfo:nil];
		[self addTrackingArea:trackingArea];
		
		NSOpenGLPixelFormat *fmt;
		
		NSOpenGLPixelFormatAttribute attrs[] =
		{
			NSOpenGLPFADoubleBuffer,
			NSOpenGLPFADepthSize, 32,
			0
		};
		
		// Init GL context
		fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes: attrs];
		m_PixelFormat = (CGLPixelFormatObj)[fmt CGLPixelFormatObj];
		g_Context = [[NSOpenGLContext alloc] initWithFormat: fmt shareContext: nil];
		[fmt release];
		[g_Context makeCurrentContext];
		
		Graphics_InitGL();
    }
    
    return self;
}

- (CGLPixelFormatObj)CGLPixelFormatObj
{
	return m_PixelFormat;
}

- (void)displayLinkCallback
{
	[self setNeedsDisplay:YES];
	
	if (g_MakeFirstResponder)
	{
		[self.window makeFirstResponder:g_View];
		g_MakeFirstResponder = false;
	}
}

// Graphics

float lastWidth = 0.f, lastHeight = 0.f;

- (void)drawRect:(NSRect)dirtyRect
{
	[g_Context setView: self];
	[g_Context makeCurrentContext];

	if (([self frame].size.width != lastWidth) || ([self frame].size.height != lastHeight))
	{
		lastWidth = [self frame].size.width;
		lastHeight = [self frame].size.height;
		
		Window_OnSizeChanged(lastWidth, lastHeight);
		
		// Only needed on resize:
		[g_Context clearDrawable];
	}
	Graphics_BeginFrame(lastWidth, lastHeight);
		
	Bacon_InternalTick();
	
	Graphics_EndFrame();
	
	[g_Context flushBuffer];
	[NSOpenGLContext clearCurrentContext];
}

// Keyboard

static int GetKeyCode(NSEvent* e)
{
	// Translate using virtual key code
	auto  it = s_VKeyCodeMap.find(e.keyCode);
	if (it != s_VKeyCodeMap.end())
		return it->second;
	
	// Translate as character
	if ([e.charactersIgnoringModifiers length] > 0)
	{
		unichar c = [e.characters characterAtIndex:0];
		auto it = s_CharMap.find(c);
		if (it != s_CharMap.end())
			return it->second;
	}
	
	return -1;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)keyDown:(NSEvent *)event {
	Keyboard_SetKeyState(GetKeyCode(event), true);
}

- (void)keyUp:(NSEvent *)event {
	Keyboard_SetKeyState(GetKeyCode(event), false);
}

- (void)flagsChanged:(NSEvent *)event {
	NSUInteger flags = event.modifierFlags;
	Keyboard_SetKeyState(Key_Alt, flags & NSAlternateKeyMask);
	Keyboard_SetKeyState(Key_Ctrl, flags & NSControlKeyMask);
	Keyboard_SetKeyState(Key_Shift, flags & NSShiftKeyMask);
	Keyboard_SetKeyState(Key_Command, flags & NSCommandKeyMask);
}

// Mouse

static void UpdateMousePosition(NSEvent* e)
{
	NSPoint p = e.locationInWindow;
	Mouse_SetMousePosition(p.x, e.window.frame.size.height - p.y);
}

- (void)mouseDown:(NSEvent *)e
{
	UpdateMousePosition(e);
	Mouse_SetMouseButtonPressed((int)e.buttonNumber, true);
}

- (void)mouseUp:(NSEvent *)e
{
	UpdateMousePosition(e);
	Mouse_SetMouseButtonPressed((int)e.buttonNumber, false);
}

- (void)mouseDragged:(NSEvent *)e
{
	UpdateMousePosition(e);
}

- (void)mouseMoved:(NSEvent *)e
{
	UpdateMousePosition(e);
}

- (void)scrollWheel:(NSEvent *)e
{
	Mouse_OnMouseScrolled(e.deltaX, e.deltaY);
}


@end

// Keymap

static struct {
	unichar m_Char;
	int m_KeyCode;
} s_CharMapEntries[] = {
	{ ' ', Key_Space },
	{ 'a', Key_A },
	{ 'b', Key_B },
	{ 'c', Key_C },
	{ 'd', Key_D },
	{ 'e', Key_E },
	{ 'f', Key_F },
	{ 'g', Key_G },
	{ 'h', Key_H },
	{ 'i', Key_I },
	{ 'j', Key_J },
	{ 'k', Key_K },
	{ 'l', Key_L },
	{ 'm', Key_M },
	{ 'n', Key_N },
	{ 'o', Key_O },
	{ 'p', Key_P },
	{ 'q', Key_Q },
	{ 'r', Key_R },
	{ 's', Key_S },
	{ 't', Key_T },
	{ 'u', Key_U },
	{ 'v', Key_V },
	{ 'w', Key_W },
	{ 'x', Key_X },
	{ 'y', Key_Y },
	{ 'z', Key_Z },
	{ 'A', Key_A },
	{ 'B', Key_B },
	{ 'C', Key_C },
	{ 'D', Key_D },
	{ 'E', Key_E },
	{ 'F', Key_F },
	{ 'G', Key_G },
	{ 'H', Key_H },
	{ 'I', Key_I },
	{ 'J', Key_J },
	{ 'K', Key_K },
	{ 'L', Key_L },
	{ 'M', Key_M },
	{ 'N', Key_N },
	{ 'O', Key_O },
	{ 'P', Key_P },
	{ 'Q', Key_Q },
	{ 'R', Key_R },
	{ 'S', Key_S },
	{ 'T', Key_T },
	{ 'U', Key_U },
	{ 'V', Key_V },
	{ 'W', Key_W },
	{ 'X', Key_X },
	{ 'Y', Key_Y },
	{ 'Z', Key_Z },
	{ ',', Key_Comma },
	{ '.', Key_Period },
	{ '/', Key_Slash },
	{ '\\', Key_Backslash },
	{ '[', Key_LeftBracket },
	{ ']', Key_RightBracket },
	{ '(', Key_LeftParen },
	{ ')', Key_RightParen },
	{ '{', Key_LeftBrace },
	{ '}', Key_RightBrace },
	{ '-', Key_Minus },
	{ '+', Key_Plus },
	{ '_', Key_Underscore },
	{ '=', Key_Equals },
	{ '`', Key_Backtick },
	{ '~', Key_Tilde },
	{ '?', Key_Question },
	{ '0', Key_Digit0 },
	{ '1', Key_Digit1 },
	{ '2', Key_Digit2 },
	{ '3', Key_Digit3 },
	{ '4', Key_Digit4 },
	{ '5', Key_Digit5 },
	{ '6', Key_Digit6 },
	{ '7', Key_Digit7 },
	{ '8', Key_Digit8 },
	{ '9', Key_Digit9 },
	{ NSLeftArrowFunctionKey, Key_Left },
	{ NSRightArrowFunctionKey, Key_Right },
	{ NSUpArrowFunctionKey, Key_Up },
	{ NSDownArrowFunctionKey, Key_Down },
	{ NSInsertFunctionKey, Key_Insert },
	{ NSDeleteFunctionKey, Key_Delete },
	{ NSHomeFunctionKey, Key_Home },
	{ NSEndFunctionKey, Key_End },
	{ NSPageUpFunctionKey, Key_PageUp },
	{ NSPageDownFunctionKey, Key_PageDown },
	{ NSF1FunctionKey, Key_F1 },
	{ NSF2FunctionKey, Key_F2 },
	{ NSF3FunctionKey, Key_F3 },
	{ NSF4FunctionKey, Key_F4 },
	{ NSF5FunctionKey, Key_F5 },
	{ NSF6FunctionKey, Key_F6 },
	{ NSF7FunctionKey, Key_F7 },
	{ NSF8FunctionKey, Key_F8 },
	{ NSF9FunctionKey, Key_F9 },
	{ NSF10FunctionKey, Key_F10 },
	{ NSF11FunctionKey, Key_F11 },
	{ NSF12FunctionKey, Key_F12 },
};

struct {
	int m_VKey;
	int m_KeyCode;
} s_VKeyCodeMapEntries[] = {
	{ kVK_Delete, Key_Backspace },
	{ kVK_Return, Key_Enter },
	{ kVK_Tab, Key_Tab },
	{ kVK_Escape, Key_Escape },
	{ kVK_ANSI_0, Key_Digit0 },
	{ kVK_ANSI_1, Key_Digit1 },
	{ kVK_ANSI_2, Key_Digit2 },
	{ kVK_ANSI_3, Key_Digit3 },
	{ kVK_ANSI_4, Key_Digit4 },
	{ kVK_ANSI_5, Key_Digit5 },
	{ kVK_ANSI_6, Key_Digit6 },
	{ kVK_ANSI_7, Key_Digit7 },
	{ kVK_ANSI_8, Key_Digit8 },
	{ kVK_ANSI_9, Key_Digit9 },
	{ kVK_ANSI_Keypad0, Key_NumPad0 },
	{ kVK_ANSI_Keypad1, Key_NumPad1 },
	{ kVK_ANSI_Keypad2, Key_NumPad2 },
	{ kVK_ANSI_Keypad3, Key_NumPad3 },
	{ kVK_ANSI_Keypad4, Key_NumPad4 },
	{ kVK_ANSI_Keypad5, Key_NumPad5 },
	{ kVK_ANSI_Keypad6, Key_NumPad6 },
	{ kVK_ANSI_Keypad7, Key_NumPad7 },
	{ kVK_ANSI_Keypad8, Key_NumPad8 },
	{ kVK_ANSI_Keypad9, Key_NumPad9 },
	{ kVK_ANSI_KeypadDivide, Key_NumPadDiv },
	{ kVK_ANSI_KeypadMultiply, Key_NumPadMul },
	{ kVK_ANSI_KeypadMinus, Key_NumPadSub },
	{ kVK_ANSI_KeypadPlus, Key_NumPadAdd },
	{ kVK_ANSI_KeypadEnter, Key_NumPadEnter },
	{ kVK_ANSI_KeypadDecimal, Key_NumPadPeriod },
};

static void InitKeyMap()
{
	for (auto& entry : s_CharMapEntries)
		s_CharMap[entry.m_Char] = entry.m_KeyCode;
	
	for (auto& entry : s_VKeyCodeMapEntries)
		s_VKeyCodeMap[entry.m_VKey] = entry.m_KeyCode;
}