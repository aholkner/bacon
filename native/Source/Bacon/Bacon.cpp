#include "Bacon.h"
#include "BaconInternal.h"

static Bacon_TickCallback s_TickCallback;

int Bacon_Init()
{
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
