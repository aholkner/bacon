#include "Bacon.h"
#include "BaconInternal.h"

#include <stdarg.h>
#include <stdio.h>

static int s_LogLevel = Bacon_LogLevel_Info;
static Bacon_LogCallback s_LogCallback = nullptr;
static Bacon_TickCallback s_TickCallback = nullptr;

int Bacon_Init()
{
    Bacon_Log(Bacon_LogLevel_Info, "Bacon %d.%d.%d", BACON_VERSION_MAJOR, BACON_VERSION_MINOR, BACON_VERSION_PATCH);
	Window_Init();
	Keyboard_Init();
	Mouse_Init();
	Graphics_Init();
	Fonts_Init();
	Audio_Init();
	Controller_Init();
	return Bacon_Error_None;
}

int Bacon_Shutdown()
{
	Controller_Shutdown();
	Audio_Shutdown();
	Fonts_Shutdown();
	Graphics_Shutdown();
	Mouse_Shutdown();
	Keyboard_Shutdown();
	Window_Shutdown();
	return Bacon_Error_None;
}

int Bacon_InternalTick()
{
	Audio_Update();
    Controller_Update();
	
	if (s_TickCallback)
		s_TickCallback();
	
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

    char buffer[1024];

    va_list args;
    va_start(args, message);
#ifdef WIN32
    vsnprintf_s(buffer, BACON_ARRAY_COUNT(buffer), _TRUNCATE, message, args);
#else
    vsnprintf(buffer, BACON_ARRAY_COUNT(buffer), message, args);
#endif
    va_end(args);

    s_LogCallback(level, buffer);
}