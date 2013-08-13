#include "../Bacon.h"
#include "../BaconInternal.h"
#include "Platform.h"
#include "Controller.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <string>
#include <unordered_map>
using namespace std;

HWND g_hWnd = NULL;
LPCTSTR WndClass = TEXT("BaconWnd");
static int s_Width = -1;
static int s_Height = -1;
static bool s_Fullscreen = false;
static bool s_Resizable = false;
static string s_Title;

static WINDOWPLACEMENT g_SavedWindowPlacement;

EGLDisplay g_Display;
EGLContext g_Context;
EGLSurface g_Surface;

static unordered_map<UINT, int> s_KeyMap;

static void InitKeyMap();
static void OnSize(int width, int height);
static void OnKey(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void OnMouseButton(HWND hWnd, UINT uMsg, WPARAM wParam, short x, short y);

static void GetWindowFrameSizeForContentSize(int& width, int& height, int windowStyle)
{
    RECT windowRect;
    windowRect.left = 0;
    windowRect.top = 0;
    windowRect.right = width;
    windowRect.bottom = height;
    AdjustWindowRect(&windowRect, windowStyle, FALSE);
    width = windowRect.right - windowRect.left;
    height = windowRect.bottom - windowRect.top;
}
    
int Bacon_SetWindowSize(int width, int height)
{
    s_Width = width;
    s_Height = height;
    if (g_hWnd)
    {
        GetWindowFrameSizeForContentSize(width, height, GetWindowLong(g_hWnd, GWL_STYLE));
        SetWindowPos(g_hWnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
    }
    return Bacon_Error_None;
}

DWORD GetWindowStyle()
{
    int style = WS_OVERLAPPEDWINDOW;
    if (!s_Resizable)
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    return style;
}

static void SetWindowFullscreen(bool fullscreen)
{
    // http://blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx
    DWORD dwStyle = GetWindowLong(g_hWnd, GWL_STYLE);
    if (fullscreen) 
    {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(g_hWnd, &g_SavedWindowPlacement) &&
            GetMonitorInfo(MonitorFromWindow(g_hWnd,
            MONITOR_DEFAULTTOPRIMARY), &mi)) 
        {
                SetWindowLong(g_hWnd, GWL_STYLE,
                    dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(g_hWnd, HWND_TOP,
                    mi.rcMonitor.left, mi.rcMonitor.top,
                    mi.rcMonitor.right - mi.rcMonitor.left,
                    mi.rcMonitor.bottom - mi.rcMonitor.top,
                    SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } 
    else 
    {
        SetWindowLong(g_hWnd, GWL_STYLE,
            dwStyle | GetWindowStyle());
        SetWindowPlacement(g_hWnd, &g_SavedWindowPlacement);
        SetWindowPos(g_hWnd, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

int Bacon_SetWindowTitle(const char* title)
{
    s_Title = title;
    if (g_hWnd)
        SetWindowText(g_hWnd, title);
    return Bacon_Error_None;
}

int Bacon_SetWindowResizable(int resizable)
{
    s_Resizable = resizable != 0;
    if (g_hWnd && !s_Fullscreen)
    {
        DWORD style = GetWindowLong(g_hWnd, GWL_STYLE);
        SetWindowLong(g_hWnd, GWL_STYLE, (style & ~WS_OVERLAPPEDWINDOW) | GetWindowStyle());
    }
    return Bacon_Error_None;
}

int Bacon_SetWindowFullscreen(int fullscreen)
{
    if ((fullscreen != 0) == s_Fullscreen)
        return Bacon_Error_None;

    s_Fullscreen = fullscreen != 0;
    if (g_hWnd)
        SetWindowFullscreen(fullscreen != 0);
    return Bacon_Error_None;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    switch (uMsg) 
    { 
    case WM_CREATE:
        break;

    case WM_SIZE:
        OnSize(lParam & 0xffff, (lParam >> 16) & 0xffff);
        // Fallthrough

    case WM_PAINT:
        if (g_Context)
        {
            Graphics_BeginFrame(s_Width, s_Height);
            Bacon_InternalTick();
            Graphics_EndFrame();

            eglSwapBuffers(g_Display, g_Surface);
            ValidateRect(g_hWnd, NULL);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);             
        break; 

    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
        OnKey(hWnd, uMsg, wParam, lParam);
        return 0;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        Mouse_SetMousePosition((float)LOWORD(lParam), (float)HIWORD(lParam));
        OnMouseButton(hWnd, uMsg, wParam, LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_MOUSEMOVE:
        Mouse_SetMousePosition((float)LOWORD(lParam), (float)HIWORD(lParam));
        return 0;

    case WM_MOUSEWHEEL:
        Mouse_SetMousePosition((float)LOWORD(lParam), (float)HIWORD(lParam));
        Mouse_OnMouseScrolled(0.f, (float)HIWORD(wParam) / WHEEL_DELTA);
        return 0;

    case WM_INPUT_DEVICE_CHANGE:
        Controller_EnumDevices();
        return 0;

    default: 
        return DefWindowProc (hWnd, uMsg, wParam, lParam); 
    } 

    return 1; 
}

static int Platform_CreateWindow()
{
    WNDCLASS wndclass = { 0 }; 
    DWORD    wStyle   = 0;
    HINSTANCE hInstance = GetModuleHandle(NULL);

    wndclass.style         = CS_OWNDC;
    wndclass.lpfnWndProc   = (WNDPROC)WndProc; 
    wndclass.hInstance     = hInstance; 
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); 
    wndclass.lpszClassName = WndClass;
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    if (!RegisterClass (&wndclass) ) 
        return FALSE; 

    wStyle = GetWindowStyle();

    // Adjust the window rectangle so that the client area has
    // the correct number of pixels
    int windowWidth = s_Width;
    int windowHeight = s_Height;
    GetWindowFrameSizeForContentSize(windowWidth, windowHeight, wStyle);
    
    g_hWnd = CreateWindow(
        WndClass,
        s_Title.c_str(),
        wStyle,
        0,
        0,
        windowWidth,
        windowHeight,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!g_hWnd)
        return Bacon_Error_Unknown;
    
    if (s_Fullscreen)
        SetWindowFullscreen(true);

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
    g_Display = eglGetDisplay(EGL_D3D11_ELSE_D3D9_DISPLAY_ANGLE);//GetDC(g_hWnd));
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

static void LogVersionInfo()
{
    OSVERSIONINFO info;
    ZeroMemory(&info, sizeof(OSVERSIONINFO));
    info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx(&info))
    {
        Bacon_Log(Bacon_LogLevel_Info, "Windows %d.%d.%d %s", 
            info.dwMajorVersion,
            info.dwMinorVersion,
            info.dwBuildNumber,
            info.szCSDVersion);
    }

    typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
    typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

    SYSTEM_INFO si;
    ZeroMemory(&si, sizeof(SYSTEM_INFO));
    PGNSI pGNSI = (PGNSI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
    if (NULL != pGNSI)
        pGNSI(&si);
    else 
        GetSystemInfo(&si);

    const char* archName = "Unknown";
    switch (si.wProcessorArchitecture)
    {
        case PROCESSOR_ARCHITECTURE_AMD64: archName = "x64"; break;
        case PROCESSOR_ARCHITECTURE_ARM: archName = "ARM"; break;
        case PROCESSOR_ARCHITECTURE_IA64: archName = "IA64"; break;
        case PROCESSOR_ARCHITECTURE_INTEL: archName = "x86"; break;
    }

    Bacon_Log(Bacon_LogLevel_Info, "Architecture: %s", archName);
    Bacon_Log(Bacon_LogLevel_Info, "Number of processors: %u", si.dwNumberOfProcessors); 
}

int Bacon_Run()
{
    AllocConsole();
    LogVersionInfo();
    InitKeyMap();

    if (int error = Platform_CreateWindow())
        return error;

    if (int error = Platform_CreateEGLContext())
        return error;

    Controller_RegisterDeviceNotifications();
    Controller_EnumDevices();
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

static void OnSize(int width, int height)
{
    if (!IsIconic(g_hWnd))
    {
        s_Width = width;
        s_Height = height;
        Window_OnSizeChanged(s_Width, s_Height);
    }
}

static struct {
    UINT m_VirtualKey;
    int m_Key;
} s_KeyMapItems[] = {
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
    { VK_OEM_MINUS, Key_Minus },
    { VK_OEM_PLUS, Key_Plus },
    { VK_OEM_COMMA, Key_Comma },
    { VK_OEM_PERIOD, Key_Period },
    { VK_OEM_1, Key_Semicolon },
    { VK_OEM_2, Key_Slash },
    { VK_OEM_3, Key_Backtick },
    { VK_OEM_4, Key_LeftBracket },
    { VK_OEM_5, Key_Backslash },
    { VK_OEM_6, Key_RightBracket },
    { VK_OEM_7, Key_Quote },
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
    { VK_ESCAPE, Key_Escape },
    { VK_SPACE, Key_Space },
    { VK_LEFT, Key_Left },
    { VK_RIGHT, Key_Right },
    { VK_UP, Key_Up },
    { VK_DOWN, Key_Down },
    { VK_RETURN, Key_Enter },
    { VK_CONTROL, Key_Ctrl },
    { VK_LCONTROL, Key_Ctrl },
    { VK_RCONTROL, Key_Ctrl },
    { VK_SHIFT, Key_Shift },
    { VK_LSHIFT, Key_Shift },
    { VK_RSHIFT, Key_Shift },
    { VK_LMENU, Key_Alt },
    { VK_RMENU, Key_Alt },
    { VK_TAB, Key_Tab },
    { VK_INSERT, Key_Insert },
    { VK_DELETE, Key_Delete },
    { '\b', Key_Backspace },
    { VK_HOME, Key_Home },
    { VK_END, Key_End },
    { VK_PRIOR, Key_PageUp },
    { VK_NEXT, Key_PageDown },
    { VK_F1, Key_F1 },
    { VK_F2, Key_F2 },
    { VK_F3, Key_F3 },
    { VK_F4, Key_F4 },
    { VK_F5, Key_F5 },
    { VK_F6, Key_F6 },
    { VK_F7, Key_F7 },
    { VK_F8, Key_F8 },
    { VK_F9, Key_F9 },
    { VK_F10, Key_F10 },
    { VK_F11, Key_F11 },
    { VK_F12, Key_F12 },
    { VK_NUMPAD0, Key_NumPad0 },
    { VK_NUMPAD1, Key_NumPad1 },
    { VK_NUMPAD2, Key_NumPad2 },
    { VK_NUMPAD3, Key_NumPad3 },
    { VK_NUMPAD4, Key_NumPad4 },
    { VK_NUMPAD5, Key_NumPad5 },
    { VK_NUMPAD6, Key_NumPad6 },
    { VK_NUMPAD7, Key_NumPad7 },
    { VK_NUMPAD8, Key_NumPad8 },
    { VK_NUMPAD9, Key_NumPad9 },
    { VK_DIVIDE, Key_NumPadDiv },
    { VK_MULTIPLY, Key_NumPadMul },
    { VK_SUBTRACT, Key_NumPadSub },
    { VK_ADD, Key_NumPadAdd },
    { VK_DECIMAL, Key_NumPadPeriod }
};

static void InitKeyMap()
{
    s_KeyMap.reserve(BACON_ARRAY_COUNT(s_KeyMapItems));
    for (size_t i = 0; i < BACON_ARRAY_COUNT(s_KeyMapItems); ++i)
        s_KeyMap[s_KeyMapItems[i].m_VirtualKey] = s_KeyMapItems[i].m_Key;
}

static void OnKey(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Ignore key repeats
    if (uMsg == WM_KEYDOWN && (lParam & BIT(30)))
        return;

    bool pressed = uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN;
    auto it = s_KeyMap.find(wParam);
    if (it == s_KeyMap.end())
    {
        UINT vkey = MapVirtualKey((lParam >> 16) & 0xff, MAPVK_VSC_TO_VK_EX);
        it = s_KeyMap.find(vkey);
    }
    if (it != s_KeyMap.end())
        Keyboard_SetKeyState(it->second, pressed);
}

static void OnMouseButton(HWND hWnd, UINT uMsg, WPARAM wParam, short x, short y)
{
    int button = 0;
    bool pressed = false;
    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
        button = 0; 
        pressed = true;
        break;
    case WM_MBUTTONDOWN:
        button = 1; 
        pressed = true;
        break;
    case WM_RBUTTONDOWN:
        button = 2; 
        pressed = true;
        break;
    case WM_LBUTTONUP:
        button = 0; 
        pressed = false;
        break;
    case WM_MBUTTONUP:
        button = 1; 
        pressed = false;
        break;
    case WM_RBUTTONUP:
        button = 2; 
        pressed = false;
        break;
    }
    Mouse_SetMouseButtonPressed(button, pressed);
}