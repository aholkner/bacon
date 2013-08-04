#include "../Bacon.h"
#include "../BaconInternal.h"
#include "Platform.h"
#include <EGL/egl.h>

HWND g_hWnd = NULL;
LPCTSTR WndClass = TEXT("BaconWnd");
int g_Width = 640, g_Height = 480;

EGLDisplay g_Display;
EGLContext g_Context;
EGLSurface g_Surface;

LRESULT WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    switch (uMsg) 
    { 
    case WM_CREATE:
        break;

    case WM_SIZE:
        // Fallthrough

    case WM_PAINT:
        if (g_Context)
        {
            Graphics_BeginFrame(g_Width, g_Height);
            Bacon_InternalTick();
            Graphics_EndFrame();

            eglSwapBuffers(g_Display, g_Surface);
            ValidateRect(g_hWnd, NULL);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);             
        break; 

    case WM_CHAR:
        // TODO
        break;

    default: 
        return DefWindowProc (hWnd, uMsg, wParam, lParam); 
    } 

    return 1; 
}


static int Platform_CreateWindow()
{
    WNDCLASS wndclass = { 0 }; 
    DWORD    wStyle   = 0;
    RECT     windowRect;
    HINSTANCE hInstance = GetModuleHandle(NULL);

    wndclass.style         = CS_OWNDC;
    wndclass.lpfnWndProc   = (WNDPROC)WndProc; 
    wndclass.hInstance     = hInstance; 
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); 
    wndclass.lpszClassName = WndClass;
    if (!RegisterClass (&wndclass) ) 
        return FALSE; 

    wStyle = WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION | WS_SIZEBOX;

    // Adjust the window rectangle so that the client area has
    // the correct number of pixels
    windowRect.left = 0;
    windowRect.top = 0;
    windowRect.right = g_Width;
    windowRect.bottom = g_Height;
    AdjustWindowRect ( &windowRect, wStyle, FALSE );

    g_hWnd = CreateWindow(
        WndClass,
        TEXT("Bacon"),
        wStyle,
        0,
        0,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!g_hWnd)
        return Bacon_Error_Unknown;
    
    return Bacon_Error_None;
}

static int Platform_CreateEGLContext()
{
    EGLint numConfigs;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLConfig config;
    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
    EGLint configAttribList[] =
    {
        EGL_RED_SIZE,       8,
        EGL_GREEN_SIZE,     8,
        EGL_BLUE_SIZE,      8,
        EGL_ALPHA_SIZE,     EGL_DONT_CARE,
        EGL_DEPTH_SIZE,     EGL_DONT_CARE,
        EGL_STENCIL_SIZE,   EGL_DONT_CARE,
        EGL_SAMPLE_BUFFERS, 0,
        EGL_NONE,           EGL_NONE
    };
    EGLint surfaceAttribList[] =
    {
        EGL_NONE, EGL_NONE
    };

    // Get Display
    g_Display = eglGetDisplay(GetDC(g_hWnd));
    if (g_Display == EGL_NO_DISPLAY)
        return Bacon_Error_Unknown;

    // Initialize EGL
    if (!eglInitialize(g_Display, &majorVersion, &minorVersion))
        return Bacon_Error_Unknown;

    // Get configs
    if (!eglGetConfigs(g_Display, NULL, 0, &numConfigs))
        return Bacon_Error_Unknown;

    // Choose config
    if (!eglChooseConfig(g_Display, configAttribList, &config, 1, &numConfigs))
        return Bacon_Error_Unknown;

    // Create a surface
    g_Surface = eglCreateWindowSurface(g_Display, config, (EGLNativeWindowType)g_hWnd, surfaceAttribList);
    if (g_Surface == EGL_NO_SURFACE)
        return Bacon_Error_Unknown;

    // Create a GL context
    g_Context = eglCreateContext(g_Display, config, EGL_NO_CONTEXT, contextAttribs);
    if (g_Context == EGL_NO_CONTEXT)
        return Bacon_Error_Unknown;

    // Make the context current
    if (!eglMakeCurrent(g_Display, g_Surface, g_Surface, g_Context))
        return Bacon_Error_Unknown;

    return Bacon_Error_None;
}

int Bacon_Run()
{
    AllocConsole();

    if (int error = Platform_CreateWindow())
        return error;

    if (int error = Platform_CreateEGLContext())
        return error;

    Graphics_InitGL();

    ShowWindow(g_hWnd, TRUE);

    MSG msg = { 0 };
 
    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0)
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            else
            {
                TranslateMessage(&msg); 
                DispatchMessage(&msg); 
            }
        }
        else
        {
            SendMessage(g_hWnd, WM_PAINT, 0, 0 );
        }
    }

    return Bacon_Error_None;
}


