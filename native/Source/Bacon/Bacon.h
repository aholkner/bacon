#pragma once

#define BACON_VERSION_MAJOR 0
#define BACON_VERSION_MINOR 1
#define BACON_VERSION_PATCH 0

typedef void (*Bacon_TickCallback)();
typedef void (*Bacon_WindowResizeEventHandler)(int width, int height);
typedef void (*Bacon_KeyEventHandler)(int key, int pressed);
typedef void (*Bacon_MouseButtonEventHandler)(int button, int pressed);
typedef void (*Bacon_MouseScrollEventHandler)(float dx, float dy);
typedef void (*Bacon_ControllerConnectedEventHandler)(int controller, int connected);
typedef void (*Bacon_ControllerButtonEventHandler)(int controller, int button, int pressed);
typedef void (*Bacon_ControllerAxisEventHandler)(int controller, int axis, float value);
typedef void (*Bacon_VoiceCallback)();

#define BIT(x) (1 << x)

enum Bacon_Error
{
	Bacon_Error_None,
	Bacon_Error_Unknown,
	Bacon_Error_InvalidArgument,
	Bacon_Error_InvalidHandle,
	Bacon_Error_StackUnderflow,
	Bacon_Error_UnsupportedFormat,
	Bacon_Error_ShaderCompileError,
	Bacon_Error_ShaderLinkError,
	Bacon_Error_NotRendering,
	Bacon_Error_InvalidFontSize,
	Bacon_Error_NotLooping
};

enum Bacon_Controller_Buttons
{
	Bacon_Controller_Button_Start = BIT(0),
	Bacon_Controller_Button_Back = BIT(1),
	Bacon_Controller_Button_Select = BIT(2),
	
	Bacon_Controller_Button_ActionUp = BIT(3),
	Bacon_Controller_Button_ActionDown = BIT(4),
	Bacon_Controller_Button_ActionLeft = BIT(5),
	Bacon_Controller_Button_ActionRight = BIT(6),

	Bacon_Controller_Button_DpadUp = BIT(7),
	Bacon_Controller_Button_DpadDown = BIT(8),
	Bacon_Controller_Button_DpadLeft = BIT(9),
	Bacon_Controller_Button_DpadRight = BIT(10),
	Bacon_Controller_ButtonMask_Dpad =
		Bacon_Controller_Button_DpadUp |
		Bacon_Controller_Button_DpadDown |
		Bacon_Controller_Button_DpadLeft |
		Bacon_Controller_Button_DpadRight,
	
	Bacon_Controller_Button_LeftShoulder = BIT(11),
	Bacon_Controller_Button_RightShoulder = BIT(12),

	Bacon_Controller_Button_LeftThumb = BIT(13),
	Bacon_Controller_Button_RightThumb = BIT(14),
	
	Bacon_Controller_Button_Misc0 = BIT(16),
};

enum Bacon_Controller_Axes
{
	Bacon_Controller_Axis_LeftThumbX = BIT(0),
	Bacon_Controller_Axis_LeftThumbY = BIT(1),
	
	Bacon_Controller_Axis_RightThumbX = BIT(2),
	Bacon_Controller_Axis_RightThumbY = BIT(3),
	
	Bacon_Controller_Axis_LeftTrigger = BIT(4),
	Bacon_Controller_Axis_RightTrigger = BIT(5),
	
	Bacon_Controller_Axis_Misc0 = BIT(8),
};

enum Bacon_Controller_Properties
{
	Bacon_Controller_Property_VendorId,
	Bacon_Controller_Property_VendorIdSource,
	Bacon_Controller_Property_ProductId,
	Bacon_Controller_Property_VersionNumber,
	Bacon_Controller_Property_Manufacturer,
	Bacon_Controller_Property_Product,
	Bacon_Controller_Property_SerialNumber,
	Bacon_Controller_Property_SupportedAxesMask,
	Bacon_Controller_Property_SupportedButtonsMask,
};

enum Bacon_Blend
{
	Bacon_Blend_Zero,
	Bacon_Blend_One,
	Bacon_Blend_SrcColor,
	Bacon_Blend_OneMinusSrcColor,
	Bacon_Blend_DstColor,
	Bacon_Blend_OneMinusDstColor,
	Bacon_Blend_SrcAlpha,
	Bacon_Blend_OneMinusSrcAlpha,
	Bacon_Blend_DstAlpha,
	Bacon_Blend_OneMinusDstAlpha,
};

enum Bacon_ImageFlags
{
	Bacon_ImageFlags_PremultiplyAlpha = 1 << 0,
	Bacon_ImageFlags_DiscardBitmap = 1 << 1
};

enum Bacon_SoundFlags
{
	Bacon_SoundFlags_Stream = 1 << 0,
	
	Bacon_SoundFlags_FormatWav = 1 << 16,
	Bacon_SoundFlags_FormatOgg = 2 << 16,
	Bacon_SoundFlags_FormatMask = 0x7fff0000
};

enum Bacon_VoiceFlags
{
	Bacon_VoiceFlags_Loop = 1 << 0,
};

enum Keys
{
	Key_None,
	Key_Space = ' ',
	Key_A = 'a',
	Key_B = 'b',
	Key_C = 'c',
	Key_D = 'd',
	Key_E = 'e',
	Key_F = 'f',
	Key_G = 'g',
	Key_H = 'h',
	Key_I = 'i',
	Key_J = 'j',
	Key_K = 'k',
	Key_L = 'l',
	Key_M = 'm',
	Key_N = 'n',
	Key_O = 'o',
	Key_P = 'p',
	Key_Q = 'q',
	Key_R = 'r',
	Key_S = 's',
	Key_T = 't',
	Key_U = 'u',
	Key_V = 'v',
	Key_W = 'w',
	Key_X = 'x',
	Key_Y = 'y',
	Key_Z = 'z',
	Key_Comma = ',',
	Key_Period = '.',
	Key_Slash = '/',
	Key_Backtick = '`',
	Key_LeftParen = '(',
	Key_RightParen = ')',
	Key_LeftBrace = '{',
	Key_RightBrace = '}',
	Key_LeftBracket = '[',
	Key_RightBracket = ']',
	Key_Backslash = '\\',
	Key_Minus = '-',
	Key_Plus = '+',
	Key_Underscore = '_',
	Key_Equals = '=',
	Key_Question = '?',
	Key_Tilde,
	Key_Digit0 = '0',
	Key_Digit1 = '1',
	Key_Digit2 = '2',
	Key_Digit3 = '3',
	Key_Digit4 = '4',
	Key_Digit5 = '5',
	Key_Digit6 = '6',
	Key_Digit7 = '7',
	Key_Digit8 = '8',
	Key_Digit9 = '9',
	Key_Left = 0x100,
	Key_Right,
	Key_Up,
	Key_Down,
	Key_Enter,
	Key_Ctrl,
	Key_Shift,
	Key_Alt,
	Key_Command,
	Key_Tab,
	Key_Insert,
	Key_Delete,
	Key_Backspace,
	Key_Home,
	Key_End,
	Key_PageUp,
	Key_PageDown,
	Key_Escape,
	Key_F1,
	Key_F2,
	Key_F3,
	Key_F4,
	Key_F5,
	Key_F6,
	Key_F7,
	Key_F8,
	Key_F9,
	Key_F10,
	Key_F11,
	Key_F12,
	Key_NumPad0,
	Key_NumPad1,
	Key_NumPad2,
	Key_NumPad3,
	Key_NumPad4,
	Key_NumPad5,
	Key_NumPad6,
	Key_NumPad7,
	Key_NumPad8,
	Key_NumPad9,
	Key_NumPadDiv,
	Key_NumPadMul,
	Key_NumPadSub,
	Key_NumPadAdd,
	Key_NumPadEnter,
	Key_NumPadPeriod,
	
	Key_MaxKey
};

#if __cplusplus
extern "C" {
#endif
	
	int Bacon_GetVersion(int* major, int* minor, int* patch);
	int Bacon_Init();
	int Bacon_Run();
	int Bacon_Shutdown();
	int Bacon_InternalTick();
	int Bacon_SetTickCallback(Bacon_TickCallback callback);

	int Bacon_SetWindowResizeEventHandler(Bacon_WindowResizeEventHandler handler);
	int Bacon_GetWindowSize(int* width, int* height);
	int Bacon_SetWindowSize(int width, int height);
	int Bacon_SetWindowFullscreen(int fullscreen);
	
	int Bacon_CreateShader(int* outHandle, const char* vertexSource, const char* fragmentSource);
	int Bacon_CreateImage(int* outHandle, int width, int height);
	int Bacon_LoadImage(int* outHandle, const char* path, int flags);
	int Bacon_UnloadImage(int handle);
	int Bacon_GetImageSize(int handle, int* width, int* height);
	
	int Bacon_PushTransform();
	int Bacon_PopTransform();
	int Bacon_Translate(float x, float y);
	int Bacon_Scale(float sx, float sy);
	int Bacon_Rotate(float radians);
	int Bacon_SetTransform(float* matrix);
	
	int Bacon_PushColor();
	int Bacon_PopColor();
	int Bacon_SetColor(float r, float g, float b, float a);
	int Bacon_MultiplyColor(float r, float g, float b, float a);
	
	int Bacon_Flush();
	int Bacon_Clear(float r, float g, float b, float a);
	int Bacon_SetFrameBuffer(int image);
	int Bacon_SetViewport(float x, float y, float width, float height);
	int Bacon_SetShader(int shader);
	int Bacon_SetBlending(int src, int dest);
	int Bacon_DrawImage(int handle, float x1, float y1, float x2, float y2);
	int Bacon_DrawImageRegion(int image, float x1, float y1, float x2, float y2,
							 float ix1, float iy1, float ix2, float iy2);
	int Bacon_DrawImageQuad(int image, float* positions, float* texCoords, float* colors);
	int Bacon_DrawLine(float x1, float y1, float x2, float y2);
	
	// Fonts
	int Bacon_LoadFont(int* outHandle, const char* path);
	int Bacon_UnloadFont(int font);
	int Bacon_GetFontMetrics(int font, float size, float* outAscent, float* outDescent);
	int Bacon_GetGlyph(int font, float size, int character, int* outImage,
					  float* outOffsetX, float* outOffsetY, float* outAdvance);

	
	// Keyboard
	int Bacon_GetKeyState(int key, int* outPressed);
	int Bacon_SetKeyEventHandler(Bacon_KeyEventHandler handler);
	
	// Mouse
	int Bacon_GetMousePosition(float* outX, float* outY);
	int Bacon_SetMouseButtonEventHandler(Bacon_MouseButtonEventHandler handler);
	int Bacon_SetMouseScrollEventHandler(Bacon_MouseScrollEventHandler handler);

	// Controller
	int Bacon_SetControllerConnectedEventHandler(Bacon_ControllerConnectedEventHandler handler);
	int Bacon_SetControllerButtonEventHandler(Bacon_ControllerButtonEventHandler handler);
	int Bacon_SetControllerAxisEventHandler(Bacon_ControllerAxisEventHandler handler);
	int Bacon_GetControllerPropertyInt(int controller, int property, int* outValue);
	int Bacon_GetControllerPropertyString(int controller, int property, char* outBuffer, int* inOutBufferSize);
	
	// Audio
	int Bacon_LoadSound(int* outHandle, const char* path, int flags);
	int Bacon_UnloadSound(int sound);
	int Bacon_PlaySound(int soundHandle);
	
	int Bacon_CreateVoice(int* outHandle, int sound, int voiceFlags);
	int Bacon_DestroyVoice(int voice);
	int Bacon_PlayVoice(int voice);
	int Bacon_StopVoice(int voice);
	int Bacon_SetVoiceGain(int voice, float gain);
	int Bacon_SetVoicePitch(int voice, float pitch);
	int Bacon_SetVoicePan(int voice, float pan);
	int Bacon_SetVoiceLoopPoints(int voice, int startSample, int endSample);
	int Bacon_SetVoiceCallback(int voice, Bacon_VoiceCallback callback);
	int Bacon_IsVoicePlaying(int voice, int* playing);
	int Bacon_GetVoicePosition(int voice, int* sample);
	int Bacon_SetVoicePosition(int voice, int sample);
	
#if __cplusplus
}
#endif