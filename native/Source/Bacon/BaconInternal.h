#pragma once

#define BACON_ARRAY_COUNT(x) \
    (sizeof(x) / sizeof(x[0]))

void Bacon_Log(Bacon_LogLevel level, const char* message, ...);

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
void Graphics_InitGL();
void Graphics_ShutdownGL();
void Graphics_BeginFrame(int width, int height);
void Graphics_EndFrame();
int Graphics_GetImageBitmap(int handle, FIBITMAP** bitmap);
int Graphics_SetImageBitmap(int handle, FIBITMAP* bitmap);
void Graphics_DrawDebugOverlay();

void Keyboard_Init();
void Keyboard_Shutdown();
void Keyboard_SetKeyState(int key, bool value);

void Mouse_Init();
void Mouse_Shutdown();

void Mouse_SetMousePosition(float x, float y);
void Mouse_SetMouseButtonPressed(int button, bool value);
void Mouse_OnMouseScrolled(float dx, float dy);

void Platform_Init();
void Platform_Shutdown();
int Platform_Run();
void Platform_Stop();

void Window_Init();
void Window_Shutdown();
void Window_OnSizeChanged(int width, int height);

