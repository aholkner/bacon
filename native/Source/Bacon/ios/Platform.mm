#include "../Bacon.h"
#include "../BaconInternal.h"
#include "Platform.h"
#include "AppDelegate.h"

#include <mach/mach.h>
#include <mach/mach_time.h>

static uint64_t s_PerformanceStartTime;
static float s_PerformanceTimebase;

void Platform_Init()
{
	s_PerformanceStartTime = mach_absolute_time();
	mach_timebase_info_data_t timebase;
	mach_timebase_info(&timebase);
	s_PerformanceTimebase = (float)timebase.numer / timebase.denom / 1000000000.f;

	Window_SetDeviceContentScale([UIScreen mainScreen].scale);
}

void Platform_Shutdown()
{ }

void Platform_GetPerformanceTime(float& time)
{
	uint64_t elapsed = mach_absolute_time() - s_PerformanceStartTime;
	time = elapsed * s_PerformanceTimebase;
}

int Platform_Run()
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	[AppDelegate staticInit];
	
	char* argv = (char*)"bacon";
	int retVal = UIApplicationMain(1, &argv, nil, @"AppDelegate");
	[pool release];
	return retVal;
}

void Platform_Stop()
{
	exit(0);
}

int Bacon_SetWindowFullscreen(int fullscreen)
{
	// Ignore
	return Bacon_Error_None;
}

int Bacon_SetWindowResizable(int resizable)
{
	// Ignore
	return Bacon_Error_None;
}

int Bacon_SetWindowSize(int width, int height)
{
	// Ignore
	return Bacon_Error_None;
}