#pragma once

#define BACON_ARRAY_COUNT(x) \
    (sizeof(x) / sizeof(x[0]))

enum RunningState
{
    RunningState_None,
    RunningState_Running,
    RunningState_Stopping,
};

void Bacon_Log(Bacon_LogLevel level, const char* message, ...);
RunningState Bacon_GetRunningState();

void Audio_Init();
void Audio_Shutdown();
void Audio_Update();

void Controller_Init();
void Controller_Shutdown();
void Controller_Update();

void Fonts_Init();
void Fonts_Shutdown();

struct FIBITMAP;
void Graphics_Init();
void Graphics_Shutdown();
void Graphics_InitGL(unsigned int defaultFramebuffer = 0);
void Graphics_ShutdownGL();
void Graphics_BeginFrame(int width, int height);
void Graphics_EndFrame();
int Graphics_GetImageBitmap(int handle, FIBITMAP** bitmap);
int Graphics_SetImageBitmap(int handle, FIBITMAP* bitmap);

void Keyboard_Init();
void Keyboard_Shutdown();
void Keyboard_SetKeyState(int key, bool value);

void Mouse_Init();
void Mouse_Shutdown();

void Mouse_SetMousePosition(float x, float y);
void Mouse_SetMouseButtonPressed(int button, bool value);
void Mouse_OnMouseScrolled(float dx, float dy);

void Touch_Init();
void Touch_Shutdown();
void Touch_SetTouchState(int touch, int state, float x, float y);

void Platform_Init();
void Platform_Shutdown();
int Platform_Run();
void Platform_Stop();
void Platform_GetPerformanceTime(float& time);

void Window_Init();
void Window_Shutdown();
void Window_OnSizeChanged(int width, int height);
void Window_SetDeviceContentScale(float contentScale);

void Debug_Init();
void Debug_Shutdown();

void DebugOverlay_Init();
void DebugOverlay_Shutdown();
void DebugOverlay_Toggle();
void DebugOverlay_Draw();
int DebugOverlay_CreateCounter(const char* label);
int DebugOverlay_CreateFloatCounter(const char* label);
void DebugOverlay_SetCounter(int counter, float value);
void DebugOverlay_AddCounter(int counter, float value);