#include "Bacon.h"
#include "BaconInternal.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static int s_LogLevel = Bacon_LogLevel_Info;
static Bacon_LogCallback s_LogCallback = nullptr;
static Bacon_TickCallback s_TickCallback = nullptr;

static RunningState s_RunningState = RunningState_None;

int Bacon_Init()
{
    if (s_RunningState != RunningState_None)
        return Bacon_Error_Running;

    Bacon_Log(Bacon_LogLevel_Info, "Bacon %d.%d.%d", BACON_VERSION_MAJOR, BACON_VERSION_MINOR, BACON_VERSION_PATCH);
    Debug_Init();
	Window_Init();
	Keyboard_Init();
	Mouse_Init();
	Touch_Init();
	Graphics_Init();
	Fonts_Init();
	Audio_Init();
	Controller_Init();
    Platform_Init();
    DebugOverlay_Init();
	return Bacon_Error_None;
}

int Bacon_Shutdown()
{
    if (s_RunningState != RunningState_None)
        return Bacon_Error_Running;

    DebugOverlay_Shutdown();
    Platform_Shutdown();
	Controller_Shutdown();
	Audio_Shutdown();
	Fonts_Shutdown();
	Graphics_Shutdown();
	Touch_Shutdown();
	Mouse_Shutdown();
	Keyboard_Shutdown();
	Window_Shutdown();
    Debug_Shutdown();
	return Bacon_Error_None;
}

RunningState Bacon_GetRunningState()
{
	return s_RunningState;
}

int Bacon_Run()
{
    if (s_RunningState != RunningState_None)
        return Bacon_Error_Running;

    s_RunningState = RunningState_Running;
    int result = Platform_Run();
    s_RunningState = RunningState_None;

    return result;
}

int Bacon_Stop()
{
    s_RunningState = RunningState_Stopping;
    Platform_Stop();
    return Bacon_Error_None;
}

int Bacon_InternalTick()
{
    if (s_RunningState != RunningState_Running)
        return Bacon_Error_None;

	Audio_Update();
    Controller_Update();
	
	if (s_TickCallback)
		s_TickCallback();
	
    DebugOverlay_Draw();

	return Bacon_Error_None;
}

int Bacon_GetVersion(int* major, int* minor, int* patch)
{
	if (!major || !minor || !patch)
		return Bacon_Error_InvalidArgument;

	*major = BACON_VERSION_MAJOR;
	*minor = BACON_VERSION_MINOR;
	*patch = BACON_VERSION_PATCH;
	return Bacon_Error_None;
}

int Bacon_SetTickCallback(Bacon_TickCallback callback)
{
	s_TickCallback = callback;
	return Bacon_Error_None;
}

int Bacon_SetLogCallback(Bacon_LogCallback callback)
{
    s_LogCallback = callback;
    return Bacon_Error_None;
}

int Bacon_SetLogLevel(int level)
{
    switch (level)
    {
    case Bacon_LogLevel_Trace:
    case Bacon_LogLevel_Info:
    case Bacon_LogLevel_Warning:
    case Bacon_LogLevel_Error:
    case Bacon_LogLevel_Fatal:
    case Bacon_LogLevel_Disable:
        s_LogLevel = level;
        return Bacon_Error_None;
    default:
        return Bacon_Error_InvalidArgument;
    }
}

void Bacon_Log(Bacon_LogLevel level, const char* message, ...)
{
    if (!s_LogCallback)
        return;

    if (s_LogLevel > level)
        return;

    char staticBuffer[64];
    char* allocatedBuffer = nullptr;
    char* buffer = &staticBuffer[0];

    va_list args;
    va_start(args, message);

#ifdef WIN32
    int bufferSize = _vscprintf(message, args) + 1;
    if (bufferSize > BACON_ARRAY_COUNT(staticBuffer))
        allocatedBuffer = buffer = (char*)malloc(bufferSize);
    vsnprintf_s(buffer, bufferSize, _TRUNCATE, message, args);
#else
    int bufferSize = vsnprintf(buffer, BACON_ARRAY_COUNT(staticBuffer), message, args) + 1;
    if (bufferSize >= BACON_ARRAY_COUNT(staticBuffer))
    {
        allocatedBuffer = buffer = (char*)malloc(bufferSize);
		va_end(args);
		va_start(args, message);
        vsnprintf(buffer, bufferSize, message, args);
    }
#endif

    va_end(args);

    s_LogCallback(level, buffer);

    if (allocatedBuffer)
        free(allocatedBuffer);
}
